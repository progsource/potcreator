# Copyright © 2021 Petra Baranski (progsource)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the “Software”), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
## This module adds the option to get translation keys from SQLite databases.
import json
import os
import db_sqlite
import strutils
import sequtils

import potc_config
import potc_output

type
  SQLiteTable = object
    name* : string
    columns* : seq[string]

  SQLiteDb = object
    path* : string
    tables* : seq[SQLiteTable]

  SQLiteConfig = ref object
    dbs* : seq[SQLiteDb]


const MODULE_NAME : string = "sqlite"


proc getModuleConfig(cfg : Config): SQLiteConfig =
  if MODULE_NAME notin cfg.modules:
    return

  result = SQLiteConfig.new

  for module in cfg.moduleData:
    doAssert module.kind == JObject
    doAssert module.hasKey("name")
    doAssert module["name"].kind == JString

    if module["name"].getStr() != MODULE_NAME:
      continue

    doAssert module.hasKey("data")
    doAssert module["data"].kind == JObject
    doAssert module["data"].hasKey("dbs")
    doAssert module["data"]["dbs"].kind == JArray

    for path in module["data"]["dbs"]:
      doAssert path.kind == JObject
      doAssert path.hasKey("path")
      doAssert path["path"].kind == JString
      doAssert path.hasKey("tables")
      doAssert path["tables"].kind == JArray

      var db : SQLiteDb
      db.path = path["path"].getStr()

      for table in path["tables"]:
        doAssert table.kind == JObject
        doAssert table.hasKey("tablename")
        doAssert table["tablename"].kind == JString
        doAssert table.hasKey("columns")
        doAssert table["columns"].kind == JArray

        var t : SQLiteTable
        t.name = table["tablename"].getStr()

        for column in table["columns"]:
          doAssert column.kind == JString
          t.columns.add(column.getStr())

        db.tables.add(t)

      result.dbs.add(db)


proc getTranslationsFromDb(basePath : string, db : SQLiteDb) : seq[Output] =
  let filename = joinPath(basePath, db.path)
  if not fileExists filename:
    return

  let dbconnection = open(filename, "", "", "")

  for table in db.tables:
    let sqlSelect = sql(join(["SELECT ", join(table.columns, ", "), " FROM ", table.name], ""))
    for row in dbconnection.fastRows(sqlSelect):
      var i : int = 0
      for column in row:
        var translation = Output.new
        translation.key = column
        translation.extract.add(join([db.path, " table: ", table.name, " column: ", table.columns[i]], ""))
        i += 1
        result.add(translation)

  dbconnection.close()


proc getTranslations*(cfg : Config): seq[Output] =
  ## Takes the potcreator config and outputs a sequence of translations
  let sqliteCfg = getModuleConfig(cfg)

  if sqliteCfg.isNil or len(sqliteCfg.dbs) == 0:
    return

  for db in sqliteCfg.dbs:
    if cfg.isVerbose:
      echo "sqlite get translations from: ", db.path
    var translations = getTranslationsFromDb(cfg.basePath, db)
    result.insert(translations, len(result))
