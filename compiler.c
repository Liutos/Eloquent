/*
 * compiler.c
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"
#include "compiler.h"
#include "misc.h"
#include "prims.h"
#include "utils/hash_table.h"
#include "utils/string.h"
#include "utils/vector.h"
#include "value.h"

typedef int (*compiler_rt_t)(compiler_t *, ast_t *, ins_t *);

/* PRIVATE */

static bytecode_t *compiler_mklabel(compiler_t *comp)
{
    char name[32] = {0};
    snprintf(name, sizeof(name), "LABEL%d", comp->counter);
    bytecode_t *label = bc_label_new(name);
    comp->counter++;
    return label;
}

static int compiler_extend_scope(compiler_t *comp, ast_t *pars)
{
    int i = 0;
    comp->env = env_new(comp->env);
    while (pars->kind == AST_CONS) {
        ast_t *par = AST_CONS_CAR(pars);
        env_push(comp->env, AST_IDENT_NAME(par), NULL, NULL, NULL);
        pars = AST_CONS_CDR(pars);
        i++;
    }
    return i;
}

static void compiler_exit_scope(compiler_t *comp)
{
    comp->env = comp->env->outer;
}

static compiler_rt_t compiler_getrt(compiler_t *comp, const char *name)
{
    return hash_table_get(comp->rts, (void *)name, NULL);
}

static void compiler_setrt(compiler_t *comp, const char *name, compiler_rt_t rt)
{
    hash_table_set(comp->rts, (void *)name, rt);
}

/* ASSEMBLER BEGIN */

static void compiler_assembly_scan(compiler_t *comp, ins_t *ins)
{
    int i = 0, offset = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        if (BC_OPCODE(bc) == BC_LABEL)
            hash_table_set(comp->label_table, BC_LABEL_NAME(bc), (void *)offset);
        else
            offset++;
        i++;
    }
}

static ins_t *compiler_assembly_rebuild(compiler_t *comp, ins_t *ins)
{
    ins_t *asm_ins = ins_new();
    int i = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        if (BC_OPCODE(bc) == BC_FJUMP) {
            BC_FJUMP_INDEX(bc) = (int)hash_table_get(comp->label_table, BC_FJUMP_LABEL_NAME(bc), NULL);
            ins_push(asm_ins, bc);
        } else if (BC_OPCODE(bc) == BC_JUMP) {
            BC_JUMP_INDEX(bc) = (int)hash_table_get(comp->label_table, BC_JUMP_LABEL_NAME(bc), NULL);
            ins_push(asm_ins, bc);
        } else if (BC_OPCODE(bc) != BC_LABEL)
            ins_push(asm_ins, bc);
        i++;
    }
    return asm_ins;
}

static ins_t *compiler_assemby(compiler_t *comp, ins_t *ins)
{
    compiler_assembly_scan(comp, ins);
    return compiler_assembly_rebuild(comp, ins);
}

/* ASSEMBLER END */

static int compiler_do_int(compiler_t *comp, ast_t *n, ins_t *ins)
{
    assert(ins != NULL);
    value_t *v = value_int_new(AST_INT_VALUE(n));
    ins_push(ins, bc_push_new(v));
    return 1;
}

static int compiler_do_ident(compiler_t *comp, ast_t *id, ins_t *ins)
{
    int i = 0, j = 0;
    char *name = AST_IDENT_NAME(id);
    /* In current lexical environment */
    if (env_locate(comp->env, name, &i, &j) == OK) {
        ins_push(ins, bc_ref_new(i, j, name));
        return OK;
    }
    /* In global environment */
    if (env_locate(comp->global_env, name, &i, &j) == OK) {
        ins_push(ins, bc_gref_new(i, j, name));
        return OK;
    }
    /* Undefined */
    string_printf(comp->error, "Line %d, column %d: Can't find value of `%s'", id->line, id->column, name);
    return ERR;
}

static int compiler_do_call(compiler_t *comp, ast_t *op, ast_t *args, ins_t *ins)
{
    int nargs = 0;
    while (args->kind == AST_CONS) {
        ast_t *expr = AST_CONS_CAR(args);
        if (compiler_do(comp, expr, ins) == ERR)
            return ERR;
        args = AST_CONS_CDR(args);
        nargs++;
    }
    if (compiler_do(comp, op, ins) == ERR)
        return ERR;
    ins_push(ins, bc_call_new(nargs));
    ins_push(ins, bc_chkex_new());
    return OK;
}

static int compiler_do_cons(compiler_t *comp, ast_t *cons, ins_t *ins)
{
    ast_t *op = AST_CONS_CAR(cons);
    if (op->kind != AST_IDENTIFIER) {
        string_printf(comp->error, "Line %d, column %d: The first element must be an identifier: %d", op->line, op->column, op->kind);
        return 0;
    }

    char *name = AST_IDENT_NAME(op);
    compiler_rt_t rt = compiler_getrt(comp, name);
    if (rt == NULL) {
        return compiler_do_call(comp, op, AST_CONS_CDR(cons), ins);
    }
    return (*rt)(comp, AST_CONS_CDR(cons), ins);
}

static int compiler_do_begin(compiler_t *comp, ast_t *body, ins_t *ins)
{
    while (body->kind == AST_CONS && AST_CONS_CDR(body)->kind != AST_END_OF_CONS) {
        ast_t *expr = AST_CONS_CAR(body);
        if (compiler_do(comp, expr, ins) == ERR)
            return ERR;
        ins_push(ins, bc_pop_new());
        body = AST_CONS_CDR(body);
    }
    return compiler_do(comp, AST_CONS_CAR(body), ins);
}

static int compiler_do_set(compiler_t *comp, ast_t *body, ins_t *ins)
{
    ast_t *var = AST_CONS_CAR(body);
    ast_t *expr = AST_CONS_CAR( AST_CONS_CDR(body) );
    bytecode_t *bc = NULL;
    int i = 0, j = 0;
    char *name = AST_IDENT_NAME(var);
    if (env_locate(comp->env, name, &i, &j) == OK) {
        /* In the current lexical environment */
        bc = bc_set_new(i, j, name);
    } else if (env_locate(comp->global_env, name, &i, &j) == OK) {
        /* In the global environment */
        bc = bc_gset_new(i, j, name);
    } else {
        /* Define in current lexical environment */
        env_push(comp->env, name, NULL, NULL, NULL);
        bc = bc_set_new(-1, -1, name);
    }
    if (compiler_do(comp, expr, ins) == ERR)
        return ERR;
    ins_push(ins, bc);
    return OK;
}

static int compiler_do_define(compiler_t *comp, ast_t *body, ins_t *ins)
{
    /* 构造符合set语法的表达式并再次编译 */
    ast_t *name = AST_CONS_CAR(body);
    ast_t *lpart = AST_CONS_CDR(body);
    ast_t *lexpr = ast_cons_new(ast_ident_new("lambda"), lpart);
    ast_t *sexpr = ast_cons_new(name, ast_cons_new(lexpr, ast_eoc_new()));
    return compiler_do_set(comp, sexpr, ins);
}

static int compiler_do_if(compiler_t *comp, ast_t *body, ins_t *ins)
{
    ast_t *pred = AST_CONS_CAR(body);
    ast_t *p = AST_CONS_CAR( AST_CONS_CDR(body) );
    ast_t *e = AST_CONS_CAR( AST_CONS_CDR( AST_CONS_CDR(body) ) );
    if (compiler_do(comp, pred, ins) == ERR)
        return ERR;
    bytecode_t *label_else = compiler_mklabel(comp);
    bytecode_t *label_end = compiler_mklabel(comp);
    ins_push(ins, bc_fjump_new(label_else));
    if (compiler_do(comp, p, ins) == ERR)
        return ERR;
    ins_push(ins, bc_jump_new(label_end));
    ins_push(ins, label_else);
    if (compiler_do(comp, e, ins) == ERR)
        return ERR;
    ins_push(ins, label_end);
    ins_push(ins, bc_nope_new());
    return 1;
}

static int compiler_do_lambda(compiler_t *comp, ast_t *body, ins_t *ins)
{
    ast_t *pars = AST_CONS_CAR(body);
    body = AST_CONS_CDR(body);
    int arity = compiler_extend_scope(comp, pars);
    ins_t *code = ins_new();
    ins_push(code, bc_args_new(arity));
    if (compiler_do_begin(comp, body, code) == ERR) {
        compiler_exit_scope(comp);
        return ERR;
    }
    ins_push(code, bc_return_new());
    compiler_exit_scope(comp);
    code = compiler_assemby(comp, code);
    value_t *f = value_ucf_new(arity, code);
    ins_push(ins, bc_push_new(f));
    ins_push(ins, bc_func_new());
    return OK;
}

static int compiler_do_dget(compiler_t *comp, ast_t *body, ins_t *ins)
{
    ast_t *var = AST_CONS_CAR(body);
    ins_push(ins, bc_dget_new(AST_IDENT_NAME(var)));
    return OK;
}

static int compiler_do_dset(compiler_t *comp, ast_t *body, ins_t *ins)
{
    ast_t *var = AST_CONS_CAR(body);
    ast_t *expr = AST_CONS_CAR( AST_CONS_CDR(body) );
    if (compiler_do(comp, expr, ins) == ERR)
        return ERR;
    ins_push(ins, bc_dset_new(AST_IDENT_NAME(var)));
    return OK;
}

/* PUBLIC */

compiler_t *compiler_new(void)
{
    compiler_t *c = malloc(sizeof(compiler_t));
    c->error = string_new();
    /* Environment */
    c->env = env_new(env_empty_new());
    c->init_env = elo_extend_env(env_new(env_empty_new()));
    c->global_env = c->init_env;

    c->label_table = hash_table_new(hash_str, comp_str);
    c->rts = hash_table_new(hash_str, comp_str);
    compiler_setrt(c, "begin", compiler_do_begin);
    compiler_setrt(c, "set", compiler_do_set);
    compiler_setrt(c, "if", compiler_do_if);
    compiler_setrt(c, "lambda", compiler_do_lambda);
    compiler_setrt(c, "dget", compiler_do_dget);
    compiler_setrt(c, "dset", compiler_do_dset);
    compiler_setrt(c, "define", compiler_do_define);
    c->counter = 0;
    return c;
}

void compiler_free(compiler_t *c)
{
    string_free(c->error);
    hash_table_free(c->rts);
    free(c);
}

int compiler_do(compiler_t *comp, ast_t *ast, ins_t *ins)
{
    assert(ins != NULL);
    int status = ERR;
    switch (ast->kind) {
        case AST_IDENTIFIER:
            status = compiler_do_ident(comp, ast, ins);
            break;
        case AST_INTEGER:
            status = compiler_do_int(comp, ast, ins);
            break;
        case AST_CONS:
            status = compiler_do_cons(comp, ast, ins);
            break;
        default :
            string_printf(comp->error, "Don't know how to compile: %d", ast->kind);
            return ERR;
    }
    ins_t *asm_ins = compiler_assemby(comp, ins);
    /* FIXME: 此处需要一个复制操作，将汇编后的指令复制到作为结果的ins变量中 */
    memcpy(ins, asm_ins, sizeof(ins_t));
    return status;
}
