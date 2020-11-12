//
// Created by Rachel Heaton on 11/11/20.
//

#include <stdlib.h>
#include <check.h>
#include "../../src/bin/pg_autoctl/parsing.h"

START_TEST(test_scrubbed_connection_string)
{
    bool parsed;

    const char *pguri;
    KeyVal *overrides;
    URIParams *uriParameters;

    parsed = parse_pguri_info_key_vals(pguri, overrides, uriParameters);
    ck_assert(parsed);
}
END_TEST

static Suite *parsing_suite(void){
    Suite *s;
    TCase *tc_core;

    s = suite_create("Parsing");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_scrubbed_connection_string);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int no_failed = 0;
    Suite *s;
    SRunner *runner;

    s = parsing_suite();
    runner = srunner_create(s);

    srunner_run_all(runner, CK_NORMAL);
    no_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return (no_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}