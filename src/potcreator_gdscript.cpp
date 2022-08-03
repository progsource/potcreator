#include "potcreator/potcreator_gdscript.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>

#include "nlohmann/json.hpp"

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_threadpool.h"
#include "potcreator/potcreator_terminal.h"
#include "potcreator/potcreator_helper.h"

// TODO: use rxterm for output
namespace ps {
namespace potcreator {

const std::string GDScriptModule::MODULE_NAME = "gdscript";
const uint32_t GDScriptModule::TERMINAL_ID = 2;

namespace {

struct GDScriptConfig
{
  std::vector<std::string> paths;
};

GDScriptConfig getModuleConfig(const Config& cfg)
{
  GDScriptConfig gdScriptCfg;

  if (!cfg.hasModule(GDScriptModule::MODULE_NAME))
  {
    return gdScriptCfg;
  }

  std::stringstream i(cfg.moduleData.at(GDScriptModule::MODULE_NAME));
  nlohmann::json j;
  i >> j;

  assert(j.contains("paths"));
  assert(j["paths"].is_array());

  for (auto it = j["paths"].begin(); it != j["paths"].end(); ++it)
  {
    auto jsub = *it;
    assert(jsub.is_string());

    gdScriptCfg.paths.push_back(jsub.get<std::string>());
  }

  return gdScriptCfg;
}

std::vector<Output> getTranslationsFromFile(std::filesystem::path basePath, std::filesystem::path path)
{
  static const std::regex re("[^a-zA-z0-9_]+(tr\\(\"([^!\\\"\"\\)]*)\"\\))");
  static const std::string startSubString = "tr(\"";
  static const size_t startSubStringSize = startSubString.size();
  static const std::string endSubString = "\")";

  std::vector<Output> out;

  std::string line;
  std::ifstream file;
  file.open(path);

  if (!file.is_open()) {
    return out;
  }

  const std::string pathString = getDisplayPath(basePath, path);

  std::smatch m;
  int32_t lineNumber = 0;

  while(getline(file, line)) {
    lineNumber++;

    if (line[0] == '#')
    {
      continue;
    }

    std::regex_search(line, m, re);

    for (const auto& x : m)
    {
      const std::string match(x);

      if (match.empty())
      {
        continue;
      }

      if (match.substr(0, startSubStringSize) != startSubString)
      {
        continue;
      }

      size_t endSubStringPos = match.find(endSubString);
      std::string key = match.substr(startSubStringSize, endSubStringPos - startSubStringSize);

      if (key.empty())
      {
        continue;
      }

      Output o;

      o.key = key;
      o.src.push_back(pathString + ":" + std::to_string(lineNumber));
      out.push_back(o);
    }
  }

  file.close();

  {
    TerminalHandle terminal;
    terminal->incrementProgress(GDScriptModule::TERMINAL_ID);
  }

  return out;
}

} // empty namespace

GDScriptModule::~GDScriptModule()
{}

std::vector<Output> GDScriptModule::getTranslations(const Config& cfg) const
{
  GDScriptConfig gdScriptCfg = getModuleConfig(cfg);

  if (gdScriptCfg.paths.empty())
  {
    return {};
  }

  std::vector<Output> out;

  ThreadPool<std::vector<Output>, 3> pool;
  uint32_t taskCount = 0;

  for (const std::string& dirPath : gdScriptCfg.paths)
  {
    std::filesystem::path path = cfg.basePath / std::filesystem::path(dirPath).relative_path();
    path = path.make_preferred();
    assert(std::filesystem::is_directory(path));

    for (const auto& file : std::filesystem::recursive_directory_iterator(path))
    {
      if (file.is_directory())
      {
        continue;
      }

      if (file.path().extension() != ".gd")
      {
        continue;
      }

      if (cfg.isVerbose)
      {
        std::cout << "gdscript get translations from: " << file.path() << std::endl;
      }

      taskCount++;

      pool.spawn([basePath = cfg.basePath, filePath = file.path()]() {
        return getTranslationsFromFile(basePath, filePath);
      });
    }
  }

  {
    TerminalHandle terminal;

    Progress taskProgress;
    taskProgress.id = GDScriptModule::TERMINAL_ID;
    taskProgress.displayName = "GDScript Module";
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
