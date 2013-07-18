# Style Guide for Myself

## Source Code Layout

* Use two **spaces** per indentation level. No hard tabs.
* Put the statement in **switch**...**case** at the next line of **case**.
* Put the **break** statement as the last one at a single line.
* Put the left brace **{** at the same line as **if**, **while**, **switch**, **struct**, **enum**.
* Avoid trailing whitespace.
* Put the **break** out of the block in **case** clause.
* Define functions as near the position to be used as possible, unless they're placed in different files.

## Naming

* Name identifiers in English.
* Use SCREAMING\_SNEAK\_CASE for macro constants.
* Use sneak\_case for variables, functions' names and labels.
* End the name of **struct** type with suffix \_t.
* Name the constructor functions like `make_XXX` where `XXX` is the **struct** type name.

## Comments

* Write self-documenting code and ignore the rest of this section. Seriously!
* Write comments in English.
* Put the comments above the code to be commented.
* Use `TODO` to note missing features or functionality that should be added at a later date.
* Use `FIXME` to note broken code(may be a bug) that needs to be fixed.

## Errors

* Check each return values of functions from standard library that may be failed.
* Add assertions at the place where program should be terminated if errors occured.
* In the definition of primitive functions to be used in Lisp, throws exceptions or terminated the program if type errors occured.

## Memory Manaegment

* Keep the functions as pure as possible.
* Constructor functions do not copy their arguments of pointer type.
* Leave the Lisp objects to become trash and be reclaimed by the GC.
* Free the memory spaces ocupied by non-Lisp objects as quickly as possible.

## Code Position

* Include header files
* Type declarations use `typedef`
* Type definitions
* Global variable declarations
* Function prototype declarations
* Constant macro definitions
* Function macro definitions
* Global variable definitions
* Function definitions
