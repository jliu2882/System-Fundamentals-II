#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "test_common.h"

#define STANDARD_LIMITS "ulimit -t 10; ulimit -f 2000"

/*
 * The following test setup ensures (hopefully) that the contents
 * of tests/rsrc/test_dir has a standard content, which matches
 * the contents of the reference output for the tests.
 */
void setup_test_dir(void) {
    system("tar -xf tests/rsrc/test_dir.tgz -z -C tests/rsrc");
}
    

/*
 * Start the program and then trigger EOF on input.
 * The program should exit with EXIT_FAILURE.
 */
Test(base_suite, start_EOF_test, .init=setup_test_dir, .timeout=30) {
    char *name = "start_EOF";
    sprintf(program_options, "tests/rsrc/test_dir");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_FAILURE, err);
    assert_outfile_matches(name, NULL);
}

/*
 * Start the program and then execute a quit command.
 * Make sure the program exits normally and the display receives the correct commands.
 */
Test(base_suite, start_quit_test, .init=setup_test_dir, .timeout=30) {
    char *name = "start_quit";
    sprintf(program_options, "tests/rsrc/test_dir");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * Start the program and test that view mode can be entered and exited successfully,
 * with correct display.
 */
Test(base_suite, view_mode_test, .init=setup_test_dir, .timeout=30) {
    char *name = "view_mode";
    sprintf(program_options, "tests/rsrc/test_dir");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * This test runs valgrind to check for memory leaks after a simple sequence of
 * operations.
 */
Test(base_suite, valgrind_leak_test, .init=setup_test_dir, .init=setup_test_dir) {
    char *name = "valgrind_leak";
    sprintf(program_options, "tests/rsrc/test_dir");
    int err = run_using_system(name, "", "valgrind --leak-check=full --undef-value-errors=no --error-exitcode=37", STANDARD_LIMITS);
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * This test runs valgrind to check for undefined variables in a simple sequence of
 * operations.
 */
Test(base_suite, valgrind_undef_test, .init=setup_test_dir, .init=setup_test_dir) {
    char *name = "valgrind_undef";
    sprintf(program_options, "tests/rsrc/test_dir");
    int err = run_using_system(name, "", "valgrind --undef-value-errors=yes --error-exitcode=37", STANDARD_LIMITS);
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}
