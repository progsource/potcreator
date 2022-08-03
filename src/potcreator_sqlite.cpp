#include "potcreator/potcreator_sqlite.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>

#include "nlohmann/json.hpp"
#include "sqlite3.h"

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_threadpool.h"
#include "potcreator/potcreator_terminal.h"

namespace ps {
namespace potcreator {

const std::string SqliteModule::MODULE_NAME = "sqlite";
const uint32_t SqliteModule::TERMINAL_ID = 4;

namespace {

struct SqliteTable
{
  std::string name;
  std::vector<std::string> columns;
};

struct SqliteDb
{
  std::string path;
  std::vector<SqliteTable> tables;
};

struct SqliteConfig
{
  std::vector<SqliteDb> dbs;
};

class SqliteHandle
{
public:
  SqliteHandle()
    : db(nullptr)
  {

  }
  ~SqliteHandle()
  {
    closeConnection();
  }

  /**
   * open SQLite connection
   */
  int32_t open(std::filesystem::path basePath, std::filesystem::path path)
  {
    static const std::regex replaceBackslash("\\\\");

    this->dbPath = path.u8string().substr(basePath.u8string().size());
    this->dbPath = std::regex_replace(this->dbPath, replaceBackslash, "/");
    if (this->dbPath[0] == '/')
    {
      this->dbPath = this->dbPath.substr(1);
    }

    if (sqlite3_open(path.u8string().c_str(), &this->db) == SQLITE_OK)
    {
      return 0;
    }

    return -1;
  }

  // run the Sql query and return results
  std::vector<Output> query(const std::string& tableName, const std::vector<std::string>& columns)
  {
    if (!db)
    {
      return {};
    }

    const std::string sql = createQuery(tableName, columns);

    std::vector<Output> out;

    sqlite3_stmt* stmt;
    const char* pzTail;

    int err = sqlite3_prepare_v2(
      this->db,
      sql.c_str(),
      -1,
      &stmt,
      &pzTail
    );

    if (err != SQLITE_OK)
    {
      // TODO: put out in terminal
      std::cout << "some error: " << err << std::endl;
    }

    sqlite3_step(stmt);

    while (sqlite3_column_text(stmt, 0))
    {
      for (size_t i = 0; i < columns.size(); ++i)
      {
        Output o;
        o.key = std::string((char *)sqlite3_column_text(stmt, i));
        o.extract.push_back(this->dbPath + " table: " + tableName + " column: " + columns[i]);
        out.push_back(o);
      }

      sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);

    return out;
  }

private:
  std::string dbPath;
  sqlite3* db;

  std::string createQuery(const std::string& tableName, const std::vector<std::string>& columns)
  {
    std::string requestQuery = "SELECT ";

    for (const std::string& column : columns)
    {
      requestQuery += column + ", ";
    }

    requestQuery.pop_back();
    requestQuery.pop_back();

    requestQuery += " FROM " + tableName;

    return requestQuery;
  }

  void closeConnection()
  {
    if (db)
    {
      sqlite3_close(db);
      db = nullptr;
    }
  }
};

SqliteConfig getModuleConfig(const Config& cfg)
{
  SqliteConfig sqliteCfg;

  if (!cfg.hasModule(SqliteModule::MODULE_NAME))
  {
    return sqliteCfg;
  }

  std::stringstream i(cfg.moduleData.at(SqliteModule::MODULE_NAME));
  nlohmann::json j;
  i >> j;

  assert(j.contains("dbs"));
  assert(j["dbs"].is_array());

  for (auto it = j["dbs"].begin(); it != j["dbs"].end(); ++it)
  {
    auto jsub = *it;
    assert(jsub.is_object());

    assert(jsub.contains("path"));
    assert(jsub["path"].is_string());
    assert(jsub.contains("tables"));
    assert(jsub["tables"].is_array());

    SqliteDb db;
    db.path = jsub["path"].get<std::string>();

    for (auto tableIt = jsub["tables"].begin(); tableIt != jsub["tables"].end(); ++tableIt)
    {
      auto jTable = *tableIt;
      assert(jTable.is_object());

      assert(jTable.contains("tablename"));
      assert(jTable["tablename"].is_string());
      assert(jTable.contains("columns"));
      assert(jTable["columns"].is_array());

      SqliteTable table;
      table.name = jTable["tablename"].get<std::string>();

      for (auto columnIt = jTable["columns"].begin(); columnIt != jTable["columns"].end(); ++columnIt)
      {
        auto column = *columnIt;
        assert(column.is_string());
        table.columns.push_back(column.get<std::string>());
      }

      db.tables.push_back(table);
    }

    sqliteCfg.dbs.push_back(db);
  }

  return sqliteCfg;
}

std::vector<Output> getTranslationsFromDb(std::filesystem::path basePath, const SqliteDb& db)
{
  std::filesystem::path path = basePath / std::filesystem::path(db.path).relative_path();
  path = path.make_preferred();

  std::vector<Output> out;

  SqliteHandle sqliteHandle;
  if (sqliteHandle.open(basePath, path) != 0)
  {
    return out;
  }

  for (const auto& table : db.tables)
  {
    if (table.columns.empty())
    {
      continue;
    }

    const auto tableResult = sqliteHandle.query(table.name, table.columns);
    mergeOutput(out, tableResult);
  }

  return out;
}

} // empty namespace

SqliteModule::~SqliteModule()
{}

std::vector<Output> SqliteModule::getTranslations(const Config& cfg) const
{
  SqliteConfig sqliteCfg = getModuleConfig(cfg);

  if (sqliteCfg.dbs.empty())
  {
    return {};
  }

  std::vector<Output> out;

  ThreadPool<std::vector<Output>, 3> pool;
  uint32_t taskCount = 0;

  for (const SqliteDb& db : sqliteCfg.dbs)
  {
    std::filesystem::path path = cfg.basePath / std::filesystem::path(db.path).relative_path();
    path = path.make_preferred();
    assert(std::filesystem::is_regular_file(path));

    if (cfg.isVerbose)
    {
      std::cout << "sqlite get translations from: " << path << std::endl;
    }

    taskCount++;

    pool.spawn([basePath = cfg.basePath, sqliteDb = db]() {
      return getTranslationsFromDb(basePath, sqliteDb);
    });
  }

  {
    TerminalHandle terminal;

    Progress taskProgress;
    taskProgress.id = SqliteModule::TERMINAL_ID;
    taskProgress.displayName = "SQLite Module";
    taskProgress.max = taskCount;
    taskProgress.current = 0;

    terminal->addProgress(taskProgress);
  }

  pool.sync();

  for (const auto& res : pool.getResponses())
  {
    mergeOutput(out, res);
  }

  return out;
}

} // namespace potcreator
} // namespace ps
