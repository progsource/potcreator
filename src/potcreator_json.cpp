#include "potcreator/potcreator_json.h"

#include <sstream>
#include <fstream>
#include <regex>
#include <cstring>
#include <deque>

#include "nlohmann/json.hpp"

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_threadpool.h"
#include "potcreator/potcreator_terminal.h"
#include "potcreator/potcreator_helper.h"

namespace ps {
namespace potcreator {

const std::string JsonModule::MODULE_NAME = "json";
const uint32_t JsonModule::TERMINAL_ID = 5;

namespace {

struct JsonFileConfig
{
  std::vector<std::string> paths;
  std::deque<std::string> jsonPaths;
};

struct JsonConfig
{
  std::vector<JsonFileConfig> files;
};

JsonConfig getModuleConfig(const Config& cfg)
{
  JsonConfig jsonCfg;

  if (!cfg.hasModule(JsonModule::MODULE_NAME))
  {
    return jsonCfg;
  }

  std::stringstream i(cfg.moduleData.at(JsonModule::MODULE_NAME));
  nlohmann::json j;
  i >> j;

  assert(j.contains("files"));
  assert(j["files"].is_array());

  for (auto it = j["files"].begin(); it != j["files"].end(); ++it)
  {
    auto jsub = *it;
    assert(jsub.is_object());

    assert(jsub.contains("paths"));
    assert(jsub["paths"].is_array());
    assert(jsub.contains("jsonPaths"));
    assert(jsub["jsonPaths"].is_array());

    JsonFileConfig fileCfg;

    for (auto pathIt = jsub["paths"].begin(); pathIt != jsub["paths"].end(); ++pathIt)
    {
      auto path = *pathIt;
      assert(path.is_string());
      fileCfg.paths.push_back(path.get<std::string>());
    }
    for (auto jsonPathIt = jsub["jsonPaths"].begin(); jsonPathIt != jsub["jsonPaths"].end(); ++jsonPathIt)
    {
      auto jsonPath = *jsonPathIt;
      assert(jsonPath.is_string());
      fileCfg.jsonPaths.push_back(jsonPath.get<std::string>());
    }

    jsonCfg.files.push_back(fileCfg);
  }

  return jsonCfg;
}

std::deque<std::string> splitString(std::string text, std::string delimeter)
{
  std::deque<std::string> out;

  char* str = text.data();
  char* ptr;
  ptr = strtok(str, delimeter.c_str());
  while (ptr != nullptr)
  {
      out.push_back(ptr);
      ptr = strtok(nullptr, delimeter.c_str());
  }

  return out;
}

std::vector<Output> fetchElement(nlohmann::json j, std::deque<std::string> jsonPaths)
{
  if (j.is_string())
  {
    Output o;
    o.key = j.get<std::string>();
    return {o};
  }
  else if (j.is_array())
  {
    std::vector<Output> out;
    for (auto it = j.begin(); it != j.end(); ++it)
    {
      std::vector<Output> jout = fetchElement(*it, jsonPaths);
      mergeOutput(out, jout);
    }
    return out;
  }
  else if (!jsonPaths.empty() && j.is_object())
  {
    if (!j.contains(jsonPaths[0]))
    {
      return {};
    }

    auto jsub = j[jsonPaths[0]];
    jsonPaths.pop_front();

    return fetchElement(jsub, jsonPaths);
  }

  return {};
}

std::vector<Output> getTranslationsFromFile(
  std::filesystem::path basePath,
  std::filesystem::path path,
  const std::deque<std::string>& jsonPaths
)
{
  const std::string pathString = getDisplayPath(basePath, path);

  std::vector<Output> out;

  std::ifstream i(path.u8string());
  nlohmann::json j;
  i >> j;

  for (const std::string& jsonPath : jsonPaths)
  {
    const std::deque<std::string> jsonParts = splitString(jsonPath, ":");

    std::vector<Output> jsonPartsOut = fetchElement(j, jsonParts);

    for (auto& o : jsonPartsOut)
    {
      o.extract.push_back(pathString + " jsonPath: \"" + jsonPath + "\"");
    }

    mergeOutput(out, jsonPartsOut);
  }

  {
    TerminalHandle terminal;
    terminal->incrementProgress(JsonModule::TERMINAL_ID);
  }

  return out;
}

} // empty namespace

JsonModule::~JsonModule()
{}

std::vector<Output> JsonModule::getTranslations(const Config& cfg) const
{
  JsonConfig jsonCfg = getModuleConfig(cfg);

  if (jsonCfg.files.empty())
  {
    return {};
  }

  std::vector<Output> out;

  ThreadPool<std::vector<Output>, 3> pool;
  uint32_t taskCount = 0;

  for (const auto& fileCfg : jsonCfg.files)
  {
    for (const auto& dirPath : fileCfg.paths)
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

        if (file.path().extension() != ".json")
        {
          continue;
        }

        if (cfg.isVerbose)
        {
          TerminalHandle terminal;
          terminal->addInfo("json get translations from: " + file.path().u8string());
        }

        taskCount++;

        pool.spawn([basePath = cfg.basePath, filePath = file.path(), jsonPaths = fileCfg.jsonPaths]() {
          return getTranslationsFromFile(basePath, filePath, jsonPaths);
        });
      }
    }
  }

  Progress taskProgress;
  taskProgress.id = JsonModule::TERMINAL_ID;
  taskProgress.displayName = "JSON Module";
  taskProgress.max = taskCount;
  taskProgress.current = 0;

  {
    TerminalHandle terminal;
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
