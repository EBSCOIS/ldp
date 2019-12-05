#include "merge.h"
#include "names.h"
#include "util.h"

static void mergeTable(const Options& opt, const TableSchema& table,
        etymon::Postgres* db)
{
    // Update history tables.

    string historyTable;
    historyTableName(table.tableName, &historyTable);

    string sql =
        "CREATE TABLE IF NOT EXISTS " + historyTable + " (\n"
        "    id VARCHAR(65535),\n"
        "    data " + opt.dbtype.jsonType() + ",\n"
        "    updated TIMESTAMPTZ,\n"
        "    PRIMARY KEY (id, updated)\n"
        ");";
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }

    string latestHistoryTable;
    latestHistoryTableName(table.tableName, &latestHistoryTable);

    sql =
        "CREATE TEMPORARY TABLE " + latestHistoryTable + " AS\n"
        "SELECT id, data\n"
        "    FROM " + historyTable + " AS h1\n"
        "    WHERE NOT EXISTS\n"
        "      ( SELECT 1\n"
        "            FROM " + historyTable + " AS h2\n"
        "            WHERE h1.id = h2.id AND\n"
        "                  h1.updated < h2.updated\n"
        "      );";
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }

    string loadingTable;
    loadingTableName(table.tableName, &loadingTable);

    sql =
        "INSERT INTO " + historyTable + "\n"+
        "SELECT s.id, s.data, 'now'\n"+
        "    FROM " + loadingTable + " AS s\n"+
        "        LEFT JOIN " + latestHistoryTable + " AS h\n"+
        "            ON s.id = h.id\n"+
        "    WHERE h.id IS NULL OR\n"+
        "          (s.data)::VARCHAR <> (h.data)::VARCHAR;\n",
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }
}

static void dropTable(const Options& opt, const TableSchema& table,
        etymon::Postgres* db)
{
    string sql = "DROP TABLE IF EXISTS " + table.tableName + ";";
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }
}

static void placeTable(const Options& opt, const TableSchema& table,
        etymon::Postgres* db)
{
    string loadingTable;
    loadingTableName(table.tableName, &loadingTable);
    string sql =
        "ALTER TABLE " + loadingTable + " RENAME TO " + table.tableName + ";";
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }
}

static void updateStatus(const Options& opt, const TableSchema& table,
        etymon::Postgres* db)
{
    string sql =
        "DELETE FROM sys.loading WHERE table_name = '" + table.tableName + "';";
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }

    sql =
        "INSERT INTO sys.loading (table_name, updated)\n"
        "    VALUES ('" + table.tableName + "', 'now');";
    printSQL(Print::debug, opt, sql);
    { etymon::PostgresResult result(db, sql); }
}

void mergeAll(const Options& opt, Schema* schema, etymon::Postgres* db)
{
    print(Print::verbose, opt, "merging");
    for (auto& table : schema->tables) {
        if (table.skip)
            continue;
        print(Print::verbose, opt, "merging table: " + table.tableName);
        mergeTable(opt, table, db);
    }
    // Table-level exclusive locks begin here.
    print(Print::verbose, opt, "replacing tables");
    for (auto& table : schema->tables) {
        if (table.skip)
            continue;
        dropTable(opt, table, db);
        placeTable(opt, table, db);
        updateStatus(opt, table, db);
    }
}

