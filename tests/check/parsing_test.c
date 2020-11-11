/* */

/* Created by Rachel Heaton on 11/11/20. */
/* */

#include <stdlib.h>
#include <check.h>
#include "../../src/bin/pg_autoctl/parsing.h"

START_TEST(test_parse_pguri_info_key_vals)
{
	bool parsed;
	char pguri[MAXCONNINFO] =
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read";
	KeyVal overrides = { 0 };
	URIParams uriParameters = { 0 };

	parsed = parse_pguri_info_key_vals(pguri, &overrides, &uriParameters);

	ck_assert(parsed);

	ck_assert_str_eq(uriParameters.dbname, "testdb");
	ck_assert_str_eq(uriParameters.hostname, "monitor.local");
	ck_assert_str_eq(uriParameters.username, "admin");
	ck_assert_str_eq(uriParameters.port, "9999");
	ck_assert_int_eq(uriParameters.parameters.count, 2);
	ck_assert_str_eq(uriParameters.parameters.keywords[0], "password");
	ck_assert_str_eq(uriParameters.parameters.values[0], "secretpassword");
	ck_assert_str_eq(uriParameters.parameters.keywords[1], "target_session_attrs");
	ck_assert_str_eq(uriParameters.parameters.values[1], "read");
}

START_TEST(test_parse_pguri_info_key_vals_with_overrides)
{
	bool parsed;
	char pguri[MAXCONNINFO] =
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read&sslmode=initialsslmode";
	KeyVal overrides = {
		1, { "sslmode" }, { "overridensslmode" }
	};

	URIParams uriParameters = { 0 };

	parsed = parse_pguri_info_key_vals(pguri, &overrides, &uriParameters);

	ck_assert(parsed);

	ck_assert_str_eq(uriParameters.dbname, "testdb");
	ck_assert_str_eq(uriParameters.hostname, "monitor.local");
	ck_assert_str_eq(uriParameters.username, "admin");
	ck_assert_str_eq(uriParameters.port, "9999");

	ck_assert_int_eq(uriParameters.parameters.count, 3);

	ck_assert_str_eq(uriParameters.parameters.keywords[0], "password");
	ck_assert_str_eq(uriParameters.parameters.values[0], "secretpassword");

	ck_assert_str_eq(uriParameters.parameters.keywords[1], "sslmode");
	ck_assert_str_eq(uriParameters.parameters.values[1], "overridensslmode");

	ck_assert_str_eq(uriParameters.parameters.keywords[2], "target_session_attrs");
	ck_assert_str_eq(uriParameters.parameters.values[2], "read");
}

START_TEST(test_parse_and_scrub_connection_string)
{
	bool parsed;
	char pguri[MAXCONNINFO] = "postgres://admin:secretpassword@monitor.local:9999/testdb";
	char scrubbedPguri[MAXCONNINFO];

	parsed = parse_and_scrub_connection_string(pguri, scrubbedPguri);

	ck_assert(parsed);
	ck_assert_str_eq(scrubbedPguri, "postgres://admin@monitor.local:9999/testdb?");
}

START_TEST(test_parse_and_scrub_connection_string_with_parameters)
{
	bool parsed;
	char pguri[MAXCONNINFO] =
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read";
	char scrubbedPguri[MAXCONNINFO];

	parsed = parse_and_scrub_connection_string(pguri, scrubbedPguri);

	ck_assert(parsed);
	ck_assert_str_eq(scrubbedPguri,
					 "postgres://admin@monitor.local:9999/testdb?target_session_attrs=read");
}

START_TEST(test_parse_and_scrub_connection_string_failure_to_parse_totally_invalid)
{
	bool parsed;
	char pguri[MAXCONNINFO] = "unparsable";
	char scrubbedPguri[MAXCONNINFO];

	parsed = parse_and_scrub_connection_string(pguri, scrubbedPguri);

	ck_assert(!parsed);
}

START_TEST(test_parse_and_scrub_connection_string_failure_to_parse_bad_uri_param)
{
	bool parsed;
	char pguri[MAXCONNINFO] =
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read&foo=bar";
	char scrubbedPguri[MAXCONNINFO];

	parsed = parse_and_scrub_connection_string(pguri, scrubbedPguri);

	ck_assert(!parsed);
}

START_TEST(test_buildPostgresURIfromPieces)
{
	bool parsed;

	URIParams params = { 0 };
	KeyVal keyval = { 0 };
	char newPgURI[MAXCONNINFO] = { 0 };

	strcpy(keyval.keywords[0], "ssl");
	strcpy(keyval.values[0], "false");
	strcpy(keyval.keywords[1], "target_session_attrs");
	strcpy(keyval.values[1], "read");
	keyval.count = 2;

	strcpy(params.dbname, "testdb");
	strcpy(params.port, "9999");
	strcpy(params.hostname, "monitor.local");
	strcpy(params.username, "admin:secretpassword");
	params.parameters = keyval;

	parsed = buildPostgresURIfromPieces(&params, newPgURI);
	ck_assert(parsed);

	ck_assert_str_eq(
		"postgres://admin:secretpassword@monitor.local:9999/testdb?ssl=false&target_session_attrs=read",
		newPgURI
		);
}
END_TEST
START_TEST(test_buildPostgresURIfromPiecesSingleParameter)
{
	bool parsed;

	URIParams params = { 0 };
	KeyVal keyval = { 0 };
	char newPgURI[MAXCONNINFO] = { 0 };

	strcpy(keyval.keywords[0], "target_session_attrs");
	strcpy(keyval.values[0], "read");
	keyval.count = 1;

	strcpy(params.dbname, "testdb");
	strcpy(params.port, "9999");
	strcpy(params.hostname, "monitor.local");
	strcpy(params.username, "admin:secretpassword");
	params.parameters = keyval;

	parsed = buildPostgresURIfromPieces(&params, newPgURI);
	ck_assert(parsed);
	ck_assert_str_eq(
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read",
		newPgURI
		);
}
