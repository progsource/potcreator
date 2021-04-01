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
## This module provides the project configuration.
import os
import json


const CONFIG_FILE_NAME = ".potcreator.json"


type
  Config* = ref object
    ## project configuration
    basePath*: string     ## base path
    outputPath*: string   ## file to output `*.pot` file based on base path
    isDisplaySrcEnabled*: bool ## en-/disable output of where translations come from
    isVerbose*: bool      ## en-/diable verbose output
    modules*: seq[string] ## configured module names
    moduleData*: JsonNode ## module configurations


proc getConfigFromFile*(path: string): Config =
  ## Takes in the base path of the project from which the translations shall be
  ## created. It expects a `.potcreator.json` file to exist in that particular
  ## directory. It returns the project configuration as object.
  doAssert dirExists path

  var cleanPath = path

  if path[len(path) - 1] == DirSep or path[len(path) - 1] == AltSep:
    cleanPath = path.substr(0, len(path) - 2)

  result = Config.new
  result.isVerbose = false
  result.basePath = cleanPath

  let filename = joinPath(cleanPath, CONFIG_FILE_NAME)
  let jsonString: string = readFile(filename)

  doAssert len(jsonString) > 0

  let jsonObject = parseJson(jsonString)

  doAssert jsonObject.kind == JObject
  doAssert jsonObject.hasKey("output")
  doAssert jsonObject["output"].kind == JString
  doAssert jsonObject.hasKey("displaySrc")
  doAssert jsonObject["displaySrc"].kind == JBool
  doAssert jsonObject.hasKey("modules")
  doAssert jsonObject["modules"].kind == JArray

  result.outputPath = jsonObject["output"].getStr()
  result.isDisplaySrcEnabled = jsonObject["displaySrc"].getBool()

  for module in jsonObject["modules"]:
    doAssert module.kind == JObject
    doAssert module.hasKey("name")
    doAssert module["name"].kind == JString

    result.modules.add(module["name"].getStr())

  result.moduleData = jsonObject["modules"]
