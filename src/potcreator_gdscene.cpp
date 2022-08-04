#include "potcreator/potcreator_gdscene.h"

#include <fstream>
#include <regex>
#include <sstream>

#include "nlohmann/json.hpp"

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_helper.h"
#include "potcreator/potcreator_terminal.h"
#include "potcreator/potcreator_threadpool.h"

namespace ps {

namespace potcreator {

const std::string GDSceneModule::MODULE_NAME = "gdscene";
const uint32_t GDSceneModule::TERMINAL_ID = 3;

namespace {

struct GDSceneConfig
{
  std::vector<std::string> paths;
};

GDSceneConfig
getModuleConfig(const Config& cfg)
{
  GDSceneConfig gdSceneCfg;

  if (!cfg.hasModule(GDSceneModule::MODULE_NAME))
  {
    return gdSceneCfg;
  }

  std::stringstream i(cfg.moduleData.at(GDSceneModule::MODULE_NAME));
  nlohmann::json j;
  i >> j;

  assert(j.contains("paths"));
  assert(j["paths"].is_array());

  for (auto it = j["paths"].begin(); it != j["paths"].end(); ++it)
  {
    auto jsub = *it;
    assert(jsub.is_string());

    gdSceneCfg.paths.push_back(jsub.get<std::string>());
  }

  return gdSceneCfg;
}

std::vector<Output>
getTranslationsFromFile(std::filesystem::path basePath, std::filesystem::path path)
{
  static const std::regex re("^(text|title) = \"([^~].*)\"");
  static const std::vector<std::string> startSubStrings = {"text = \"", "title = \""};
  static const std::vector<size_t> startSubStringSizes = [strs = startSubStrings]()
                                                         {
                                                           std::vector<size_t> sizes;
                                                           for (const std::string& str : strs)
                                                           {
                                                             sizes.push_back(str.size());
                                                           }
                                                           return sizes;
                                                         }();

  std::vector<Output> out;

  std::string line;
  std::ifstream file;
  file.open(path);

  if (!file.is_open())
  {
    {
      TerminalHandle terminal;
      terminal->addError("Cannot open file " + path.u8string());
    }
    return out;
  }

  const std::string pathString = getDisplayPath(basePath, path);

  std::smatch m;
  int32_t lineNumber = 0;

  while (getline(file, line))
  {
    lineNumber++;

    std::regex_search(line, m, re);

    for (const auto& x : m)
    {
      const std::string match(x);

      if (match.empty())
      {
        continue;
      }

      bool has_key_without_extra = true;
      size_t key_index = 0;

      for (size_t i = 0; i < startSubStrings.size(); ++i)
      {
        if (match.substr(0, startSubStringSizes[i]) == startSubStrings[i])
        {
          has_key_without_extra = false;
          key_index = i;
          break;
        }
      }

      if (has_key_without_extra)
      {
        continue;
      }

      std::string key = match;
      key = key.substr(
        startSubStringSizes[key_index],
        key.size() - startSubStringSizes[key_index] - 1
        );

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
    terminal->incrementProgress(GDSceneModule::TERMINAL_ID);
  }

  return out;
}

} // empty namespace

GDSceneModule::~GDSceneModule()
{}

std::vector<Output>
GDSceneModule::getTranslations(const Config& cfg) const
{
  GDSceneConfig gdSceneCfg = getModuleConfig(cfg);

  if (gdSceneCfg.paths.empty())
  {
    return {};
  }

  std::vector<Output> out;

  ThreadPool<std::vector<Output>, 3> pool;
  uint32_t taskCount = 0;

  for (const std::string& dirPath : gdSceneCfg.paths)
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

      if (file.path().extension() != ".tscn")
      {
        continue;
      }

      if (cfg.isVerbose)
      {
        TerminalHandle terminal;
        terminal->addInfo("gdscene get translations from: " + file.path().u8string());
      }

      taskCount++;

      pool.spawn(
        [basePath = cfg.basePath, filePath = file.path()]()
          {
            return getTranslationsFromFile(basePath, filePath);
          }
        );
    }
  }

  Progress taskProgress;
  taskProgress.id = GDSceneModule::TERMINAL_ID;
  taskProgress.displayName = "GDScene Module";
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
