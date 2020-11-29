#!/usr/bin/env bats
#
# Unit tests for nessie
# Bats seems practical for testing status codes and output
# without having a crash bring the entire test suite down

@test "gives error 127 on invalid command" {
    run nessie -c "thiscommanddoesnotexist"
    [ "$status" -eq 127 ]
}

# Options

@test "--help shows some information" {
    run nessie --help
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ne 0 ]
    run nessie -h
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ne 0 ]
}

# Modes

@test "nessie can read stdin" {
    # Piping with the run command does not work
    # (Bats is treating the run command _itself_ as part of the pipe)
    run sh -c "echo \"echo it just works # y'know\" | nessie"
    [ "$status" -eq 0 ]
    [ "$output" = "it just works" ]
}

@test "nessie can read script files" {
    cat <<-EOF > /tmp/test.sh
	#!/usr/bin/env nessie

	echo it just works # and does not print this
	EOF
    run nessie /tmp/test.sh
    [ "$status" -eq 0 ]
    [ "$output" = "it just works" ]
}

# Builtins

@test "cd changes working directory" {
    run nessie -c "cd /tmp ; pwd"
    [ "$status" -eq 0 ]
    [ "$output" = "/tmp" ]
}

@test "exit exits the shell" {
    run nessie -c "exit ; seq 1 100"
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -eq 0 ]
}

@test "exit does not always trigger" {
    run nessie -c "false && exit ; true || exit ; echo correct"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "help shows some information" {
    run nessie -c "help"
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ne 0 ]
}

@test "history shows nothing when run non-interactively" {
    run nessie -c "history"
    [ "$status" -eq 0 ]
    [ -z "$output" ]
}

# Piping

@test "pipes output through a pair of commands" {
    run nessie -c 'printf "aaaa\nbbbb\naaaa\n" | grep a'
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "aaaa" ]
    [ "${lines[1]}" = "aaaa" ]
}

@test "pipes output through multiple commands" {
    run nessie -c 'printf "aaaa\nbbbb\naaaa\n" | grep a | tr a A'
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "AAAA" ]
    [ "${lines[1]}" = "AAAA" ]
}

@test "pipes can be squished together" {
    run nessie -c 'printf "aaaa\nbbbb\naaaa\n"|grep a|tr a A'
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "AAAA" ]
    [ "${lines[1]}" = "AAAA" ]
}

# Statements and short-circuiting

@test "handles multiple statements" {
    run nessie -c "echo one; echo two; echo three"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "one" ]
    [ "${lines[1]}" = "two" ]
    [ "${lines[2]}" = "three" ]
}

@test "|| does not abort subsequent statements" {
    run nessie -c "true || false; echo correct"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "&& can be chained; breaking the chain triggers ||" {
    run nessie -c "true && true && false && true || echo correct"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

# Parser behaviour

@test "Every operator can be squished together with text" {
    run nessie -c "true&&true&&false&&true||echo correct|tr r l;false"
    [ "$status" -eq 1 ]
    [ "$output" = "collect" ]
}

@test "Single-quoted strings exist" {
    run nessie -c "echo 'this is inside single quotes'"
    [ "$status" -eq 0 ]
    [ "$output" = "this is inside single quotes" ]
}

@test "Double-quoted strings exist" {
    run nessie -c 'echo "this is inside double quotes"'
    [ "$status" -eq 0 ]
    [ "$output" = "this is inside double quotes" ]
}

@test "Operators inside a string are interpreted literally" {
    run nessie -c "true && echo 'this || that is && fine; yes indeed'"
    [ "$status" -eq 0 ]
    [ "$output" = "this || that is && fine; yes indeed" ]
}

@test "Comments are usually skipped" {
    run nessie -c "echo this is # all well and good"
    [ "$status" -eq 0 ]
    [ "$output" = "this is" ]
}

run nessie -c "echo '#hashtag'"
@test "Comments are not skipped if they're inside a string" {
    [ "$status" -eq 0 ]
    [ "$output" = "#hashtag" ]
    run nessie -c 'echo "#hashtag"'
    [ "$status" -eq 0 ]
    [ "$output" = "#hashtag" ]
}
