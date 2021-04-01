# Package

version       = "0.2.0"
author        = "progsource"
description   = "Tool to generate pot files from various sources"
license       = "MIT"
srcDir        = "src"
installExt    = @["nim"]
bin           = @["potcreator"]



# Dependencies

requires "nim >= 1.2.6", "argparse"
