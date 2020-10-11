#!/usr/bin/env bats
#
# Unit tests for nessie
# Bats seems practical for testing status codes and output
# without having a crash bring the entire test suite down

@test "gives error 127 on invalid command" {
    run nessie -c "thiscommanddoesnotexist"
    [ "$status" -eq 127 ]
}

# Builtins

@test "cd changes working directory" {
    run nessie -c "cd /tmp ; pwd"
    [ "$status" -eq 0 ]
    [ "$output" = "/tmp" ]
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

# Statements and short-circuiting

@test "handles multiple statements" {
    run nessie -c "echo one ; echo two ; echo three"
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "one" ]
    [ "${lines[1]}" = "two" ]
    [ "${lines[2]}" = "three" ]
}

@test "|| does not abort subsequent statements" {
    run nessie -c "true || false ; echo correct"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}

@test "&& can be chained; breaking the chain triggers ||" {
    run nessie -c "true && true && false && true || echo correct"
    [ "$status" -eq 0 ]
    [ "$output" = "correct" ]
}
