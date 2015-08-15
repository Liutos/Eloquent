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

static const char *bc_names[] = {
        BC_KIND(BC_STRINGIFY)
};

static bytecode_t *bc_new(opcode_t kind)
{
    bytecode_t *bc = malloc(sizeof(bytecode_t));
    BC_OPCODE(bc) = kind;
    return bc;
}

static void ins_indent_print(int indent, int index, FILE *output)
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

static void ins_push_ucf_print(value_t *ucf, FILE *output, int indent, int index)
{
    ins_indent_print(indent, index, output);
    fprintf(output, "BC_PUSH #< ; This is a function\n");
    ins_t *ins = VALUE_UCF_CODE(ucf);
    ins_pretty_print(ins, output, indent + 1);
    ins_indent_print(indent, -1, output);
    fprintf(output, "        >");
}

static void ins_push_print(bytecode_t *bc, FILE *output, int indent, int index)
{
    value_t *object = BC_PUSH_PTR(bc);
    if (object->kind == VALUE_FUNCTION && VALUE_FUNC_ISCMP(object))
        ins_push_ucf_print(object, output, indent, index);
    else {
        ins_indent_print(indent, index, output);
        bc_print(bc, output);
    }
}

/* PUBLIC */

bytecode_t *bc_addr_new(const char *name)
{
    bytecode_t *bc = bc_new(BC_ADDR);
    BC_ADDR_VAR(bc) = ident_intern(name);
    return bc;
}

bytecode_t *bc_args_new(int arity)
{
    bytecode_t *bc = bc_new(BC_ARGS);
    bc->u.bc_args.arity = arity;
    return bc;
}

bytecode_t *bc_call_new(int nargs)
{
    bytecode_t *bc = bc_new(BC_CALL);
    BC_CALL_NARGS(bc) = nargs;
    return bc;
}

bytecode_t *bc_chkex_new(void)
{
    return bc_new(BC_CHKEX);
}

bytecode_t *bc_dget_new(char *name)
{
    bytecode_t *bc = bc_new(BC_DGET);
    BC_DGET_NAME(bc) = name;
    return bc;
}

bytecode_t *bc_dset_new(char *name)
{
    bytecode_t *bc = bc_new(BC_DSET);
    BC_DSET_NAME(bc) = name;
    return bc;
}

bytecode_t *bc_fjump_new(bytecode_t *label)
{
    bytecode_t *bc = bc_new(BC_FJUMP);
    BC_FJUMP_INDEX(bc) = -1;
    bc->u.bc_fjump.label = label;
    return bc;
}

bytecode_t *bc_func_new(void)
{
    return bc_new(BC_FUNC);
}

bytecode_t *bc_gref_new(int i, int j, char *name)
{
    bytecode_t *bc = bc_new(BC_GREF);
    BC_REF_I(bc) = i;
    BC_REF_J(bc) = j;
    BC_REF_NAME(bc) = name;
    return bc;
}

bytecode_t *bc_gset_new(int i, int j, char *name)
{
    bytecode_t *bc = bc_new(BC_GSET);
    BC_SET_I(bc) = i;
    BC_SET_J(bc) = j;
    BC_SET_NAME(bc) = name;
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
    BC_LABEL_IDENT(bc) = ident_intern(name);
    return bc;
}

bytecode_t *bc_nope_new(void)
{
    return bc_new(BC_NOPE);
}

bytecode_t *bc_pop_new(void)
{
    return bc_new(BC_POP);
}

bytecode_t *bc_print_new(void)
{
    return bc_new(BC_PRINT);
}

bytecode_t *bc_push_new(void *ptr)
{
    bytecode_t *bc = bc_new(BC_PUSH);
    BC_PUSH_PTR(bc) = ptr;
    return bc;
}

bytecode_t *bc_ref_new(int i, int j, char *name)
{
    bytecode_t *bc = bc_new(BC_REF);
    BC_REF_I(bc) = i;
    BC_REF_J(bc) = j;
    BC_REF_NAME(bc) = name;
    return bc;
}

bytecode_t *bc_return_new(void)
{
    return bc_new(BC_RETURN);
}

bytecode_t *bc_set_new(int i, int j, char *name)
{
    bytecode_t *bc = bc_new(BC_SET);
    bc->u.bc_set.i = i;
    bc->u.bc_set.j = j;
    bc->u.bc_set.name = name;
    return bc;
}

bytecode_t *bc_valof_new(void)
{
    return bc_new(BC_VALOF);
}

void bc_print(bytecode_t *bc, FILE *output)
{
    fprintf(output, "%s", bc_name(bc));
    switch (BC_OPCODE(bc)) {
        case BC_ADDR:
            fprintf(output, " %s", BC_ADDR_NAME(bc));
            break;
        case BC_ARGS:
            fprintf(output, " %d", BC_ARGS_ARITY(bc));
            break;
        case BC_CALL:
            fprintf(output, " %d", BC_CALL_NARGS(bc));
            break;
        case BC_DGET:
            fprintf(output, " %s", BC_DGET_NAME(bc));
            break;
        case BC_DSET:
            fprintf(output, " %s", BC_DSET_NAME(bc));
            break;
        case BC_FJUMP:
            fprintf(output, " %d", BC_FJUMP_INDEX(bc));
            break;
        case BC_JUMP:
            fprintf(output, " %d", BC_JUMP_INDEX(bc));
            break;
        case BC_LABEL:
            fprintf(output, " %s", BC_LABEL_NAME(bc));
            break;
        case BC_PUSH:
            fputc(' ', output);
            value_print(BC_PUSH_PTR(bc), output);
            break;
        case BC_GREF:
        case BC_REF:
            fprintf(output, " %d %d ; %s", BC_REF_I(bc), BC_REF_J(bc), BC_REF_NAME(bc));
            break;
        case BC_GSET:
        case BC_SET:
            fprintf(output, " %d %d ; %s", BC_SET_I(bc), BC_SET_J(bc), BC_SET_NAME(bc));
            break;
        default :
            break;
    }
}

void ins_pretty_print(ins_t *ins, FILE *output, int indent)
{
    int i = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        assert(BC_OPCODE(bc) != BC_LABEL);
        if (BC_OPCODE(bc) != BC_PUSH) {
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
    return bc_names[BC_OPCODE(bc)];
}
