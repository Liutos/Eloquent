# Eloquent

# Introduction

Eloquent is a reference implementation of 233-Lisp, which is a personal dialect of Lisp.

# Building

## Build The Compiler Test

    make test_compiler

## Build The Virtual Machine Test

    make test_vm

## Build The REPL

    make test_repl

# Add A New Primitive Function

## First: Define The New Function

Define the new primitive function in file `prims.c', and put its declaration into the header file `prims.h'. The data type of the formal parameters and return value of this new function must be `struct lisp\_object\_t *', which also named `lt *' by typedef.

## Second: Install The New Function

Register this new function into the implementation's global environment. In the body of function `init\_prims', use the predefined macro ADD for installing. Macro ADD has four parameters, their meaning list as follow:

1. arity. It's the total number of parameters of a primitive function. The rest parameter, for example, the parameter y in form (x . y) is also counted. Therefor, parameters list (x . y) means arity of 2.
2. restp. A flag indicates whether the function accepts variable number of parameters or not.
3. function_name. The name of the function been defined.
4. Lisp_name. It's a C string, used to be the function name in the Lisp code.

## Third: It's OK Now!

The new primitive function is invokable in the Lisp code level.
