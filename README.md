# Ne[sh]ie

_The absurdly stupid shell_

Minimal shell written in C.

## Features

+ [x] Fork and execute commands
+ [x] `cd` with builtin or by typing a folder name
+ [x] Piping
  + Currently w/o builtins
+ [x] Multiple statements terminated by `;`
  + Currently requires surrounding the semicolon with whitespace
+ [ ] Conditional execution with `&&` and `||`
+ [ ] History (at least as builtin)
+ [ ] Variables
+ [ ] Some keybinds (requires raw mode + rework of input code)

## Grammar

Nessie's grammar is pretty minimal for the time being

```
LINE      := STATEMENT [; STATEMENT]*
STATEMENT := COMMAND
           | COMMAND && STATEMENT
           | COMMAND || STATEMENT
COMMAND   := PROG [ARG]* [| COMMAND]*
PROG, ARG := WORD
           | "WORD [WORD]*"
WORD      := <just continuous text>
```

## Resources used

+ Linux manual pages for various C functions
+ _The C Programming Language_ by Brian W. Kernighan and Dennis M. Ritchie
+ [This stackoverflow answer](https://stackoverflow.com/questions/33884291/pipes-dup2-and-exec) to a question about piping in C
