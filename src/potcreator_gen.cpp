#include "potcreator/potcreator_gen.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <assert.h>

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_output.h"

namespace ps {
namespace potcreator {

void genPot(const Config& cfg, const std::vector<Output>& translations)
{
  static const std::string HEADER(
    u8"# Don't remove the two lines below, they're required for gettext to work correctly.\nmsgid \"\"\nmsgstr \"\"\n"
  );

  std::string potData = HEADER;

  for (const Output& out : translations)
  {
    if (cfg.isDisplaySrcEnabled)
    {
      for (const std::string& extract : out.extract)
      {
        potData += "\n#. " + extract;
      }

      for (const std::string& src : out.src)
      {
        potData += "\n#: " + src;
      }
    }

    potData += "\nmsgid \"" + out.key + "\"\nmsgstr \"\"\n";
  }

  std::filesystem::path outFile = cfg.basePath / cfg.outputPath.relative_path();
  outFile.make_preferred();

  assert(std::filesystem::is_directory(outFile.parent_path()));

  if (cfg.isVerbose)
  {
    std::cout << "\nfile: " << outFile << "\n" << std::endl;
    std::cout << potData << std::endl;
  }

  std::ofstream file(outFile, std::ios_base::binary | std::ios_base::out);

  if (!file.is_open())
  {
    std::cout << "could not open file" << std::endl;
    return;
  }

  file << potData;

  file.close();
}

} // namespace potcreator
} // namespace ps
