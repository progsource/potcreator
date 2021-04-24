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
## This module includes the generator for the `*.pot` file.
import algorithm
import sequtils
import strutils
import os

import potc_config
import potc_output

proc cmpTranslations(x, y: Output): int =
  if x.key < y.key: -1
  elif x.key == y.key: 0
  else: 1

proc mergeTranslations(translations: seq[Output]): seq[Output] =
  var trs = translations
  var keys: seq[string]

  var tls: seq[Output] = sorted(trs, cmpTranslations)
  for t in tls:
    if t.key notin keys:
      keys.add(t.key)
      result.add(t)
      continue

    var index = binarySearch(result, t, cmpTranslations)
    doAssert index >= 0

    result[index].src.insert(t.src, len(result[index].src))
    result[index].extract.insert(t.extract, len(result[index].extract))


proc genPot*(cfg: Config, translations: seq[Output]): int =
  ## Takes in the project configuration and the collected translations.
  ## It generates the `*.pot` file and returns 0 on success else some other int.
  const HEADER = """
# Don't remove the two lines below, they're required for gettext to work correctly.
msgid ""
msgstr ""

"""

  result = 0

  let trs = mergeTranslations(translations)
  var potData = HEADER

  for translation in trs:
    if len(translation.key) == 0:
      continue

    if cfg.isDisplaySrcEnabled:
      for extract in translation.extract:
        potData = potData & "\n#. " & extract
      for src in translation.src:
        potData = potData & "\n#: " & src
    potData = potData & "\nmsgid \"" & translation.key & "\"\nmsgstr \"\"\n"

  writeFile(joinPath(cfg.basePath, cfg.outputPath), potData)

  echo cfg.outputPath, " written successfully"
