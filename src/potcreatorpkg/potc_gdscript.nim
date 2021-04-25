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
## This module adds the option to get translation keys from GDScript files.
import os
import re
import json
import std/strutils
import sequtils
import threadpool

import potc_config
import potc_output


type
  GDScriptConfig = ref object
    paths*: seq[string]

  ThreadOut = ref object
    keys*: seq[Output]


const MODULE_NAME: string = "gdscript"


proc getModuleConfig(cfg: Config): GDScriptConfig =
  if MODULE_NAME notin cfg.modules:
    return

  result = GDScriptConfig.new

  for module in cfg.moduleData:
    doAssert module.kind == JObject
    doAssert module.hasKey("name")
    doAssert module["name"].kind == JString

    if module["name"].getStr() != MODULE_NAME:
      continue

    doAssert module.hasKey("data")
    doAssert module["data"].kind == JObject
    doAssert module["data"].hasKey("paths")
    doAssert module["data"]["paths"].kind == JArray

    for path in module["data"]["paths"]:
      doAssert path.kind == JString

      result.paths.add(path.getStr())


proc getTranslationsFromFile(basePath: string, path: string): ThreadOut =
  result = ThreadOut.new
  let filename = expandFilename(path).substr(len(expandFilename(basePath)) + 1).replace("\\", "/")

  doAssert fileExists path

  let regTest = re"[^a-zA-z0-9_]+tr\(""([^!\""\)]*)""\)"
  const startSubString = "tr(\""
  const endSubStringSize = len("\")")
  var lineNumber: int = 0

  for line in lines path:
    lineNumber += 1
    let matches = findAll(line, regTest)

    for m in matches:
      var outp = Output.new
      outp.src.add(filename & ":" & intToStr(lineNumber))

      var startSubStringPos = m.find(startSubString) + len(startSubString)
      outp.key = m.substr(startSubStringPos, len(m) - endSubStringSize - 1)
      result.keys.add(outp)


proc getTranslations*(cfg: Config): seq[Output] =
  ## Takes the potcreator config and outputs a sequence of translations
  let gdscriptConfig = getModuleConfig(cfg)

  if gdscriptConfig.isNil or len(gdscriptConfig.paths) == 0:
    return

  var responses = newSeq[FlowVar[ThreadOut]]()

  for path in gdscriptConfig.paths:
    let p = joinPath(cfg.basePath, path)
    for file in walkDirRec p:
      if file.match re".*\.gd":
        if cfg.isVerbose:
          echo "gdscript get translations from: ", file
        responses.add(spawn getTranslationsFromFile(cfg.basePath, file))
  sync()

  for r in responses:
    var outp = ThreadOut(^r)
    assert(not outp.isNil)

    if len(outp.keys) > 0:
      result.insert(outp.keys, len(result))
