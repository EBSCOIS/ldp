#include "initutil.h"

void create_main_table_sql(const string& table_name, etymon::odbc_conn* conn,
                           const dbtype& dbt, string* sql)
{
    string rskeys;
    dbt.redshift_keys("id", "id", &rskeys);
    *sql =
        "CREATE TABLE " + table_name + " (\n"
        "    id VARCHAR(65535) NOT NULL,\n"
        "    data " + dbt.json_type() + ",\n"
        "    tenant_id SMALLINT NOT NULL,\n"
        "    PRIMARY KEY (id)\n"
        ")" + rskeys + ";";
}

void create_history_table_sql(const string& table_name,
                              etymon::odbc_conn* conn, const dbtype& dbt,
                              string* sql)
{
    string rskeys;
    dbt.redshift_keys("id", "id, updated", &rskeys);
    *sql =
        "CREATE TABLE IF NOT EXISTS\n"
        "    history." + table_name + " (\n"
        "    id VARCHAR(36) NOT NULL,\n"
        "    data " + dbt.json_type() + " NOT NULL,\n"
        "    updated TIMESTAMP WITH TIME ZONE NOT NULL,\n"
        "    tenant_id SMALLINT NOT NULL,\n"
        "    CONSTRAINT\n"
        "        history_" + table_name + "_pkey\n"
        "        PRIMARY KEY (id, updated)\n"
        ")" + rskeys + ";";
}

void grant_select_on_table_sql(const string& table, const string& user,
                               etymon::odbc_conn* conn, string* sql)
{
    *sql =
        "GRANT SELECT ON " + table + "\n"
        "    TO " + user + ";";
}

void add_table_to_catalog_sql(etymon::odbc_conn* conn, const string& table,
                              string* sql)
{
    *sql =
        "INSERT INTO ldpsystem.tables (table_name) VALUES\n"
        "    ('" + table + "');";
}

