#!/usr/bin/env bats
#
# Unit tests for nessie
# Bats seems practical for testing status codes and output
# without having a crash bring the entire test suite down

@test "gives error 127 on invalid command" {
    run nessie -c "thiscommanddoesnotexist"
    echo "$output"
    [ "$status" -eq 127 ]
}

# Options

@test "--help shows some information" {
    run nessie --help
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ne 0 ]
    echo "$output"
    run nessie -h
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ne 0 ]
}

# Modes

@test "Nessie can read stdin" {
    # Piping with the run command does not work
    # (Bats is treating the run command _itself_ as part of the pipe)
    run sh -c "echo \"echo it just works # y'know\" | nessie"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "it just works" ]
}

@test "Nessie can read script files" {
    cat <<-EOF > /tmp/test.sh
	#!/usr/bin/env nessie

	echo it just works # and does not print this
	EOF
    run nessie /tmp/test.sh
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "it just works" ]
}

# Builtins

@test "cd changes working directory" {
    run nessie -c "cd /tmp ; pwd"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "/tmp" ]
}

@test "exit exits the shell" {
    run nessie -c "exit ; seq 1 100"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -eq 0 ]
}

@test "exit does not always trigger" {
    run nessie -c "false && exit ; true || exit ; echo correct"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "help shows some information" {
    run nessie -c "help"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ne 0 ]
}

@test "history shows nothing when run non-interactively" {
    run nessie -c "history"
    echo "$output"
    [ "$status" -eq 0 ]
    [ -z "$output" ]
}

# Piping

@test "Pipes output through a pair of commands" {
    run nessie -c 'printf "aaaa\nbbbb\naaaa\n" | grep a'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "aaaa" ]
    [ "${lines[1]}" = "aaaa" ]
}

@test "Pipes output through multiple commands" {
    run nessie -c 'printf "aaaa\nbbbb\naaaa\n" | grep a | tr a A'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "AAAA" ]
    [ "${lines[1]}" = "AAAA" ]
}

@test "Pipes can be squished together" {
    run nessie -c 'printf "aaaa\nbbbb\naaaa\n"|grep a|tr a A'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "AAAA" ]
    [ "${lines[1]}" = "AAAA" ]
}

# Statements and short-circuiting

@test "Handles multiple statements" {
    run nessie -c "echo one; echo two; echo three"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "one" ]
    [ "${lines[1]}" = "two" ]
    [ "${lines[2]}" = "three" ]
}

@test "|| does not abort subsequent statements" {
    run nessie -c "true || false; echo correct"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "&& can be chained; breaking the chain triggers ||" {
    run nessie -c "true && true && false && true || echo correct"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

# Variables

@test "Nessie can read environment variables" {
    SOME_VARIABLE="correct" run nessie -c 'echo $SOME_VARIABLE'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "Nessie can set environment variables" {
    # Note to self: Variables will only be usable on the next line, of course.
    cat <<-EOF > /tmp/var1.sh
	let SOME_OTHER_VARIABLE correct
	echo \$SOME_OTHER_VARIABLE
	EOF
    run nessie /tmp/var1.sh
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "Variables are expanded in double-quoted strings" {
    x=correct run nessie -c 'echo "$x"'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "Variables are not expanded in literal strings" {
    x=correct run nessie -c "echo '\$x'"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = '$x' ]
}

@test "Multi-word variables are expanded properly" {
    words="there are some words" run nessie -c 'printf "%s%s%s_%s\n" $words'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "therearesome_words" ]
    words="there are some words" run nessie -c 'printf "%s\n" "$words"'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "there are some words" ]
    danger="true && false" run nessie -c 'false || $danger'
    echo "$output"
    [ "$status" -eq 1 ]
    [ -z "$output" ]
}

@test "Variables are expanded even if they are stuck together with something" {
    # Swapped out a builtin for `ls` since _something_ is wrong elsewhere
    x=correct y="correct indeed" run nessie -c 'echo $x; echo $y&& echo'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "correct" ]
    [ "${lines[1]}" = "correct indeed" ]
}

# Parser behaviour

@test "Every operator can be squished together with text" {
    run nessie -c "true&&true&&false&&true||echo correct|tr r l;false"
    echo "$output"
    [ "$status" -eq 1 ]
    [ "$output" = "collect" ]
}

@test "Single-quoted strings exist" {
    run nessie -c "echo 'this is inside single quotes'"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "this is inside single quotes" ]
}

@test "Double-quoted strings exist" {
    run nessie -c 'echo "this is inside double quotes"'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "this is inside double quotes" ]
}

@test "Strings are properly merged with words if they are adjacent to them" {
    run nessie -c 'echo does" this work or "what'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "does this work or what" ]
    run nessie -c 'echo cat" in a hat"'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "cat in a hat" ]
    run nessie -c 'echo "cat in a "hat'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "cat in a hat" ]
    run nessie -c 'echo what"ev"er'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "whatever" ]
}

@test "Operators inside a string are interpreted literally" {
    run nessie -c "true && echo 'this || that is && fine; yes indeed'"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "this || that is && fine; yes indeed" ]
}

@test "Comments are usually skipped" {
    run nessie -c "echo this is # all well and good"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "this is" ]
}

@test "Comments are not skipped if they're inside a string" {
    run nessie -c "echo '#hashtag'"
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "#hashtag" ]
    run nessie -c 'echo "#hashtag"'
    echo "$output"
    [ "$status" -eq 0 ]
    [ "$output" = "#hashtag" ]
}
