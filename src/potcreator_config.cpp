#include "potcreator/potcreator_config.h"

#include <fstream>
#include <assert.h>
#include <algorithm>

#include "nlohmann/json.hpp"

namespace ps {
namespace potcreator {

namespace {

void checkConfigfileConstaints(nlohmann::json j)
{
  assert(j.is_object());
  assert(j.contains("output"));
  assert(j["output"].is_string());
  assert(j.contains("displaySrc"));
  assert(j["displaySrc"].is_boolean());
  assert(j.contains("modules"));
  assert(j["modules"].is_array());
}

void checkModuleConstraints(nlohmann::json jsub)
{
  assert(jsub.is_object());
  assert(jsub.contains("name"));
  assert(jsub["name"].is_string());
  assert(jsub.contains("data"));
  assert(jsub["data"].is_object());
}

} // empty namespace

bool Config::hasModule(const std::string& module) const
{
  return std::any_of(
    this->modules.begin(),
    this->modules.end(),
    [module](const std::string& any)
    {
      return module == any;
    }
  );
}

Config
getConfigFromFile(std::string path)
{
  Config cfg;
  cfg.isVerbose = false;

  std::filesystem::path projectDirectory(path);
  assert(std::filesystem::exists(projectDirectory));

  cfg.basePath = projectDirectory;

  std::filesystem::path cfgFile(projectDirectory / CONFIG_FILE_NAME);
  assert(std::filesystem::exists(cfgFile));

  std::ifstream i(cfgFile.string());
  nlohmann::json j;
  i >> j;

  checkConfigfileConstaints(j);

  cfg.outputPath = std::filesystem::path(j["output"].get<std::string>());
  cfg.isDisplaySrcEnabled = j["displaySrc"].get<bool>();

  for (auto it = j["modules"].begin(); it != j["modules"].end(); ++it)
  {
    auto jsub = *it;

    checkModuleConstraints(jsub);

    cfg.modules.push_back(jsub["name"].get<std::string>());
    cfg.moduleData[jsub["name"].get<std::string>()] = jsub["data"].dump();
  }

  return cfg;
}

} // namespace potcreator
} // namespace ps
