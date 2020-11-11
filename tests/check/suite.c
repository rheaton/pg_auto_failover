/* */

/* Created by Rachel Heaton on 11/12/20. */
/* */

#include <stdlib.h>
#include <check.h>
#include "parsing_test.c"
#include "pgsql_test.c"

static Suite *
parsing_suite(void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_pgsql;

	s = suite_create("PG Auto Failover");

	tc_core = tcase_create("parsing.c");
	tcase_add_test(tc_core, test_parse_pguri_info_key_vals);
	tcase_add_test(tc_core, test_parse_pguri_info_key_vals_with_overrides);
	tcase_add_test(tc_core, test_buildPostgresURIfromPieces);
	tcase_add_test(tc_core, test_buildPostgresURIfromPiecesSingleParameter);
	tcase_add_test(tc_core, test_parse_and_scrub_connection_string);
	tcase_add_test(tc_core, test_parse_and_scrub_connection_string_with_parameters);
	tcase_add_test(tc_core,
				   test_parse_and_scrub_connection_string_failure_to_parse_totally_invalid);
	tcase_add_test(tc_core,
				   test_parse_and_scrub_connection_string_failure_to_parse_bad_uri_param);
	suite_add_tcase(s, tc_core);

	tc_pgsql = tcase_create("pgsql.c");
	tcase_add_test(tc_pgsql, test_pgsql_init);
	tcase_add_test(tc_pgsql, test_validate_connection_string);
	tcase_add_test(tc_pgsql, test_validate_connection_string_invalid);
	tcase_add_test(tc_pgsql, test_validate_connection_string_too_long);
	suite_add_tcase(s, tc_pgsql);

	return s;
}


int
main(void)
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
