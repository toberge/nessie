# Ne[sh]ie

[![Unit tests](https://github.com/toberge/nessie/workflows/Unit%20tests/badge.svg)](https://github.com/toberge/nessie/actions?query=workflow%3A%22Unit+tests%22)
[![Fuzzer tests](https://github.com/toberge/nessie/workflows/Fuzzer%20tests/badge.svg)](https://github.com/toberge/nessie/actions?query=workflow%3A%22Fuzzer+tests%22)

_The absurdly stupid shell_

Minimal shell written in C as a hobby project.

![Demo session](res/demo.svg)

## Features

+ [x] Fork and execute commands
+ [x] `cd` with builtin or by typing a folder name
+ [x] Piping
+ [x] Multiple statements terminated by `;`
+ [x] Conditional execution with `&&` and `||`
+ [x] History (as builtin)
+ [x] Comments
+ [x] Script files (including stdin)
+ [ ] Variables
+ [ ] Some keybinds (requires raw mode + rework of input code)

## Requirements

Nessie depends on nothing but the tools required to build it – `make` and a C compiler – and some regular Linux syscalls.

Running the tests requires `bats` and/or `clang`.

Producing the man page requires `pandoc` – this is only done in the `make install` target.

## Installation

Run `make install` or `make install-local`. Alternatively, simply run `make` and put the binary wherever you want.

You should now be able to execute `nessie` as a regular command, or run the binary with `./nessie`.

## Testing

Run unit tests with `make test` or fuzzer tests with `make fuzz`.

## Usage

Start an interactive session by simply running `nessie`.
If you are stuck, try running `help`, `man nessie` or `man <some other command>`.

Run a single command with `nessie -c "some command"`.

Execute a script file with `nessie script.sh`. One can also pipe or otherwise redirect a list of commands to `nessie`, like `echo "some command" | nessie`.

See the [manual page](man.md) for a more thorough explanation of Nessie's functionality.

## Grammar

Nessie's grammar is pretty minimal for the time being.

```
LINE      := STATEMENT [; STATEMENT]* [# COMMENT]
STATEMENT := COMMAND
           | COMMAND && STATEMENT
           | COMMAND || STATEMENT
COMMAND   := PROG [ARG]* [| COMMAND]*
PROG, ARG := WORD
           | "WORD [WORD]*"
WORD      := <just continuous text>
COMMENT   := <any text following a #>
```

## Resources used

+ Linux manual pages for various C functions
+ _The C Programming Language_ by Brian W. Kernighan and Dennis M. Ritchie
+ [This stackoverflow answer](https://stackoverflow.com/questions/33884291/pipes-dup2-and-exec) to a question about piping in C
