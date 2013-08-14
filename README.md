# Eloquent

# 简介

Eloquent 是一个233-Lisp的参考实现，后者是一门个人Lisp方言。

# 作为脚本使用

将你所写的代码保存到文件中，假设这个文件名为script.scm，那么可以像下面这样加载这个文件

    ./test_repl -l script.scm

# 构建

## 构建编译器测试

    make test_compiler

## 构建虚拟机测试

    make test_vm

## 构建REPL

    make test_repl

# 添加一个新的原语函数

## 第一步：定义这个新函数

在文件 prims.c 中定义这个新的原语函数，然后将它的原型声明放到文件 prims.h 中。这个新函数的形参和返回值的类型都必须是 struct lisp\_object\_t *，也可以使用被 typedef 过的类型名 lt *。

## 第二步：安装这个新函数

在语言实现的全局环境中注册这个新的函数。在函数 init\_prims 的函数体部分，用预定义好的宏 ADD 进行安装。宏 ADD 有四个参数，它们各自的含义分别如下：

1. arity。是指一个原语函数的参数的总数。对于rest参数，例如参数列表 (x . y) 中的 y 也被计算在内。因此，参数列表 (x . y) 意味着这个原语函数的参数个数为2。
2. restp。这是一个表示一个函数是否能够接收任意多个参数的标记。
3. function_name。原语函数的名字
4. Lisp_name。这是一个 C 语言中的字符串，用来作为该函数在 Lisp 代码中的函数名。

## 第三步：现在一切就绪！

这个新的原语函数可以在 Lisp 代码中被调用了。
