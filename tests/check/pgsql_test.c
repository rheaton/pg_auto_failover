/* */

/* Created by Rachel Heaton on 11/12/20. */
/* */

#include <check.h>
#include "../../src/bin/pg_autoctl/pgsql.h"

START_TEST(test_pgsql_init)
{
	bool initialized;

	PGSQL pgsql = { 0 };
	char uri[MAXCONNINFO] =
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read";
	ConnectionType connectionType = PGSQL_CONN_LOCAL;

	initialized = pgsql_init(&pgsql, uri, connectionType);
	ck_assert(initialized);
	ck_assert_str_eq(pgsql.connectionString, uri);
}
END_TEST
START_TEST(test_validate_connection_string)
{
	bool valid;

	char uri[MAXCONNINFO] =
		"postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read";

	valid = validate_connection_string(uri);
	ck_assert(valid);
}


END_TEST
START_TEST(test_validate_connection_string_invalid)
{
	bool valid;

	char uri[MAXCONNINFO] = "invalid";

	valid = validate_connection_string(uri);
	ck_assert(!valid);
}


END_TEST
START_TEST(test_validate_connection_string_too_long)
{
	bool valid;

	char superLong[MAXCONNINFO + 2];
	strcpy(superLong, "a");
	for (int i = 0; i < MAXCONNINFO; i++)
	{
		strcat(superLong, "a");
	}


	char uri[MAXCONNINFO * 2];
	strcpy(uri,
		   "postgres://admin:secretpassword@monitor.local:9999/testdb?target_session_attrs=read&application_name=");
	strcat(uri, superLong);

	valid = validate_connection_string(uri);
	ck_assert(!valid);
}


END_TEST
