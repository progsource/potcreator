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
import os

import argparse

import potcreatorpkg/potc_config
import potcreatorpkg/potc_output
import potcreatorpkg/potc_gen

# modules
import potcreatorpkg/potc_gdscript
import potcreatorpkg/potc_sqlite


proc runCmd(): int =
  const version = "0.1.0"
  result = 0

  var cmdl: seq[TaintedString]
  when declared(commandLineParams):
    cmdl = commandLineParams()
  else:
    echo "commandLineParams not found - cannot run"
    return 1

  var p = newParser:
    help("{prog} collects translation keys from multiple resources and puts them into a *.pot file")
    flag("-v", "--verbose", help = "Verbose output")
    flag("-V", "--version", help = "Show version")
    arg("pathToConfig", help = "path to \".potcreator.json\" file")

  try:
    let args = p.parse(cmdl)
    var cfg = getConfigFromFile(args.pathToConfig)
    if args.verbose:
      cfg.isVerbose = true

    if args.verbose:
      echo "output path: ", cfg.outputPath
      echo "active modules:"
      for m in cfg.modules:
        echo "* ", m

    var translations = potc_gdscript.getTranslations(cfg)
    var sqlTranslations = potc_sqlite.getTranslations(cfg)

    translations.insert(sqlTranslations, len(translations))

    return genPot(cfg, translations)

  except ShortCircuit as e:
    if e.flag == "argparse_help":
      echo p.help
      return 0

  except UsageError:
    if "-V" in cmdl or "--version" in cmdl:
      echo "potcreator ", version
      return
    echo getCurrentExceptionMsg()
    return 1


when isMainModule:
  quit(runCmd())
