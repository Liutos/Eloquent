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
#include "prims.h"
#include "utils/hash_table.h"
#include "utils/string.h"
#include "utils/vector.h"
#include "value.h"

#define ERR 0
#define OK 1

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

static compiler_env_t *compiler_env_new(compiler_env_t *outer)
{
    return seg_vector_new(outer);
}

static void compiler_env_intern(compiler_env_t *env, const char *var, int *i, int *j)
{
    if (i != NULL)
        *i = 0;
    if (j != NULL)
        *j = env->data->count;
    seg_vector_push(env, var);
}

static int compiler_env_lookup(compiler_env_t *env, const char *var, int *i, int *j)
{
    return seg_vector_locate(env, var, (ele_comp_t)comp_str, i, j);
}

static int compiler_extend_scope(compiler_t *comp, ast_t *pars)
{
    int i = 0;
    comp->env = compiler_env_new(comp->env);
    while (pars->kind == AST_CONS) {
        ast_t *par = AST_CONS_CAR(pars);
        compiler_env_intern(comp->env, AST_IDENT_NAME(par), NULL, NULL);
        pars = AST_CONS_CDR(pars);
        i++;
    }
    return i;
}

static void compiler_exit_scope(compiler_t *comp)
{
    comp->env = comp->env->next;
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
        if (bc->kind == BC_LABEL)
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
        if (bc->kind == BC_FJUMP) {
            BC_FJUMP_INDEX(bc) = (int)hash_table_get(comp->label_table, BC_FJUMP_LABEL_NAME(bc), NULL);
            ins_push(asm_ins, bc);
        } else if (bc->kind == BC_JUMP) {
            BC_JUMP_INDEX(bc) = (int)hash_table_get(comp->label_table, BC_JUMP_LABEL_NAME(bc), NULL);
            ins_push(asm_ins, bc);
        } else if (bc->kind != BC_LABEL)
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
    if (compiler_env_lookup(comp->env, name, &i, &j) == 0) {
        string_printf(comp->error, "Line %d, column %d: Can't find value of `%s'", id->line, id->column, name);
        return ERR;
    }
    ins_push(ins, bc_get_new(i, j, name));
    return OK;
}

static int compiler_do_call(compiler_t *comp, ast_t *op, ast_t *args, ins_t *ins)
{
    while (args->kind == AST_CONS) {
        ast_t *expr = AST_CONS_CAR(args);
        if (compiler_do(comp, expr, ins) == ERR)
            return ERR;
        args = AST_CONS_CDR(args);
    }
    if (compiler_do(comp, op, ins) == ERR)
        return ERR;
    ins_push(ins, bc_call_new());
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
    int i = 0, j = 0;
    char *name = AST_IDENT_NAME(var);
    if (compiler_env_lookup(comp->env, name, &i, &j) == ERR) {
        compiler_env_intern(comp->env, name, NULL, NULL);
        i = j = -1;
    }
    if (compiler_do(comp, expr, ins) == ERR)
        return ERR;
    ins_push(ins, bc_set_new(i, j, name));
    return OK;
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

/* PUBLIC */

compiler_t *compiler_new(void)
{
    compiler_t *c = malloc(sizeof(compiler_t));
    c->error = string_new();
    c->env = compiler_env_new(NULL);
    c->label_table = hash_table_new(hash_str, comp_str);
    c->rts = hash_table_new(hash_str, comp_str);
    compiler_setrt(c, "begin", compiler_do_begin);
    compiler_setrt(c, "set", compiler_do_set);
    compiler_setrt(c, "if", compiler_do_if);
    compiler_setrt(c, "lambda", compiler_do_lambda);
    c->counter = 0;

    int i = 0;
    while (i < prims_num) {
        prim_t *p = &prims[i];
        compiler_env_intern(c->env, p->name, NULL, NULL);
        i++;
    }

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
