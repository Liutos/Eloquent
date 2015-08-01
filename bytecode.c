/*
 * bytecode.c
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"
#include "value.h"

/* PRIVATE */

static const char *bc_names[] ={
        BC_KIND(BC_STRINGIFY)
};

static bytecode_t *bc_new(opcode_t kind)
{
    bytecode_t *bc = malloc(sizeof(bytecode_t));
    bc->opcode = kind;
    return bc;
}

/* PUBLIC */

bytecode_t *bc_pop_new(void)
{
    return bc_new(BC_POP);
}

bytecode_t *bc_push_new(void *ptr)
{
    bytecode_t *bc = bc_new(BC_PUSH);
    bc->u.push_ptr = ptr;
    return bc;
}

bytecode_t *bc_get_new(int i, int j, char *name)
{
    bytecode_t *bc = bc_new(BC_GET);
    bc->u.bc_get.i = i;
    bc->u.bc_get.j = j;
    bc->u.bc_get.name = name;
    return bc;
}

bytecode_t *bc_set_new(int i, int j, char *name)
{
    bytecode_t *bc = bc_new(BC_SET);
    bc->u.bc_set.i = i;
    bc->u.bc_set.j = j;
    bc->u.bc_set.name = name;
    return bc;
}

bytecode_t *bc_fjump_new(bytecode_t *label)
{
    bytecode_t *bc = bc_new(BC_FJUMP);
    BC_FJUMP_INDEX(bc) = -1;
    bc->u.bc_fjump.label = label;
    return bc;
}

bytecode_t *bc_jump_new(bytecode_t *label)
{
    bytecode_t *bc = bc_new(BC_JUMP);
    BC_JUMP_LABEL(bc) = label;
    BC_JUMP_INDEX(bc) = -1;
    return bc;
}

bytecode_t *bc_label_new(const char *name)
{
    bytecode_t *bc = bc_new(BC_LABEL);
    bc->u.bc_label.name = string_new();
    string_assign(bc->u.bc_label.name, name);
    return bc;
}

bytecode_t *bc_nope_new(void)
{
    return bc_new(BC_NOPE);
}

bytecode_t *bc_call_new(int nargs)
{
    bytecode_t *bc = bc_new(BC_CALL);
    BC_CALL_NARGS(bc) = nargs;
    return bc;
}

bytecode_t *bc_args_new(int arity)
{
    bytecode_t *bc = bc_new(BC_ARGS);
    bc->u.bc_args.arity = arity;
    return bc;
}

bytecode_t *bc_func_new(void)
{
    return bc_new(BC_FUNC);
}

bytecode_t *bc_chkex_new(void)
{
    return bc_new(BC_CHKEX);
}

bytecode_t *bc_return_new(void)
{
    return bc_new(BC_RETURN);
}

bytecode_t *bc_print_new(void)
{
    return bc_new(BC_PRINT);
}

void bytecode_free(bytecode_t *bc)
{
    free(bc);
}

void ins_indent_print(int indent, int index, FILE *output)
{
    int prefix = indent - 1;
    while (prefix >= 0) {
        fputc('\t', output);
        prefix--;
    }
    if (index != -1)
        fprintf(output, "[%d]", index);
    fputc('\t', output);
}

void ins_push_ucf_print(value_t *ucf, FILE *output, int indent, int index)
{
    ins_indent_print(indent, index, output);
    fprintf(output, "BC_PUSH #< ; This is a function\n");
    ins_t *ins = VALUE_UCF_CODE(ucf);
    ins_pretty_print(ins, output, indent + 1);
    ins_indent_print(indent, -1, output);
    fprintf(output, "        >");
}

void ins_push_print(bytecode_t *bc, FILE *output, int indent, int index)
{
    value_t *object = BC_PUSH_OBJ(bc);
    if (object->kind == VALUE_FUNCTION && VALUE_FUNC_ISCMP(object))
        ins_push_ucf_print(object, output, indent, index);
    else {
        ins_indent_print(indent, index, output);
        bc_print(bc, output);
    }
}

void ins_pretty_print(ins_t *ins, FILE *output, int indent)
{
    int i = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        assert(bc->opcode != BC_LABEL);
        if (bc->opcode != BC_PUSH) {
            ins_indent_print(indent, i, output);
            bc_print(bc, output);
        } else
            ins_push_print(bc, output, indent, i);
        fputc('\n', output);
        i++;
    }
}

const char *bc_name(bytecode_t *bc)
{
    return bc_names[bc->opcode];
}

void bc_print(bytecode_t *bc, FILE *output)
{
    switch (bc->opcode) {
        case BC_ARGS:
            fprintf(output, "%s %d", bc_name(bc), BC_ARGS_ARITY(bc));
            break;
        case BC_CALL:
            fprintf(output, "%s %d", bc_name(bc), BC_CALL_NARGS(bc));
            break;
        case BC_FJUMP:
            fprintf(output, "%s %d", bc_name(bc), BC_FJUMP_INDEX(bc));
            break;
        case BC_JUMP:
            fprintf(output, "%s %d", bc_name(bc), BC_JUMP_INDEX(bc));
            break;
        case BC_LABEL:
            fprintf(output, "%s %s", bc_name(bc), BC_LABEL_NAME(bc));
            break;
        case BC_SET:
            fprintf(output, "%s %d %d ; %s", bc_name(bc), bc->u.bc_set.i, bc->u.bc_set.j, bc->u.bc_set.name);
            break;
        case BC_PUSH:
            fprintf(output, "%s ", bc_name(bc));
            value_print(BC_PUSH_OBJ(bc), output);
            break;
        case BC_GET:
            fprintf(output, "%s %d %d ; %s", bc_name(bc), bc->u.bc_get.i, bc->u.bc_get.j, bc->u.bc_get.name);
            break;
        case BC_POP:
        default :
            fprintf(output, "%s", bc_name(bc));
    }
}

void bc_sprint(bytecode_t *bc, char *desc, size_t size)
{
    FILE *outs = fmemopen(desc, size, "w");
    bc_print(bc, outs);
    fclose(outs);
}
