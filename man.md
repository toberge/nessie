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

TODO: Place the grammar here

Command
:    TODO

Pipeline
:    TODO

Short-circuiting
:    TODO

# EXAMPLES

1. Execute a shell command from a string:

   **nessie -c "true && false || echo whatever | tr a o"**

2. Start an interactive session

   **nessie**
