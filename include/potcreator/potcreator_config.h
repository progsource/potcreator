#ifndef PS_POTCREATOR_CONFIG_H
#define PS_POTCREATOR_CONFIG_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace ps {

namespace potcreator {

const std::string CONFIG_FILE_NAME = ".potcreator.json";

struct Config
{
  std::filesystem::path basePath;
  std::filesystem::path outputPath;
  bool isDisplaySrcEnabled;
  bool isVerbose;
  std::vector<std::string> modules;
  std::map<std::string, std::string> moduleData;

  bool hasModule(const std::string& module) const;
};

Config getConfigFromFile(std::string path);

} // namespace potcreator

} // namespace ps

#endif // PS_POTCREATOR_CONFIG_H
