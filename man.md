% NESSIE(1)
% Tore Bergebakken
% November 2020

# NAME

ne[sh]ie – the absurdly stupid shell

# SYNOPSIS

**nessie** [**-d|-\-debug**] [**-h|-\-help**] [**-c|-\-command** *COMMAND*] [*FILE*]

# DESCRIPTION

**nessie** is a simple Linux shell

# OPTIONS

**-h**, **-\-help**
:   Display a friendly help message.

**-c**, **-\-command**
:   Execute a command, or a set of commands, specified as option parameter

**-d**, **-\-debug**
:   Run the shell in debug mode – Nessie will print extra information

# SYNTAX

Nessie's syntax should be familiar to anyone with some experience using standard UNIX shells. It is meant to follow a tiny subset of Bourne Shell syntax.

The grammar can be summarized as follows:

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

**Command**
:    An exectuable file in your PATH, or a builtin. Takes some arguments and/or input, does whatever it is supposed to and returns an exit code.

**Pipeline**
:    The output of one command can be piped into another – that is, whatever the command would print if you ran it by itself, will become the input to the command after the **|** sign.

**Short-circuiting**
:    Nessie supports conditional execution, also known as short-circuiting, with the **&&** and **||** operators, which correspond to logical **AND** and **OR**. A command that exits with status code 0, triggers the next **&&**. A command that exits with a nonzero status code, triggers the next **||**.

# EXAMPLES

1. Execute a shell command from a string:

   **nessie -c "true && false || echo whatever | tr a o"**

2. Start an interactive session

   **nessie**
