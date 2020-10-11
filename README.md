# Ne[sh]ie

_The absurdly stupid shell_

Minimal shell written in C.

## Features

+ [x] Fork and execute commands
+ [x] `cd` with builtin or by typing a folder name
+ [x] Piping (currently w/o builtins)
+ [ ] History (at least as builtin)
+ [ ] Multiple statements terminated by `;`
+ [ ] Variables
+ [ ] Some keybinds (requires raw mode + rework of input code)

## Grammar

Nessie's grammar is pretty minimal for the time being

```
LINE := STATEMENT [; STATEMENT]*
STATEMENT := COMMAND [| COMMAND]*
```

## Resources used

+ Linux manual pages for various C functions
+ _The C Programming Language_ by Brian W. Kernighan and Dennis M. Ritchie
+ [This stackoverflow answer](https://stackoverflow.com/questions/33884291/pipes-dup2-and-exec) to a question about piping in C
