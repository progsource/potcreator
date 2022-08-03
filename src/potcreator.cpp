#include "potcreator/potcreator.h"

#include <iostream>
#include <filesystem>

#include "argh.h"

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_gdscript.h"
#include "potcreator/potcreator_gdscene.h"
#include "potcreator/potcreator_json.h"
#include "potcreator/potcreator_sqlite.h"
#include "potcreator/potcreator_output.h"
#include "potcreator/potcreator_gen.h"
#include "potcreator/potcreator_terminal.h"

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define OS_WIN
#endif

namespace ps {
namespace potcreator {

namespace {

static const uint32_t TERMINAL_MODULES_ID = 1;

void showVersion()
{
  std::cout << "potcreator version " << VERSION << std::endl;
}

template <typename T>
std::vector<Output> getTranslationsFromModule(Config& cfg)
{
  T module;
  return module.getTranslations(cfg);
}

void updateTerminalModuleProgress(uint32_t progress)
{
  TerminalHandle terminal;
  terminal->updateProgress(TERMINAL_MODULES_ID, progress);
}

} // empty namespace

struct PotCreator::Pimpl
{
  Config cfg;
  uint32_t modulesDone = 0;
  std::vector<Output> translations;

  template <typename T>
  void fetchTranslationsForModule()
  {
    if (cfg.hasModule(T::MODULE_NAME))
    {
      std::vector<Output> moduleTranslations = getTranslationsFromModule<T>(cfg);
      mergeOutput(translations, moduleTranslations);
      modulesDone++;

      updateTerminalModuleProgress(modulesDone);
    }
  }
};

PotCreator::PotCreator()
  : pimpl(std::make_unique<PotCreator::Pimpl>())
{}

PotCreator::~PotCreator() {}

int PotCreator::run(int argc, char** argv)
{
  // for rxterm to work on most things, this seems to be necessary
  #ifdef OS_WIN
    system(" ");
  #endif

  argh::parser cmdl;
  cmdl.add_params({"-h", "--help", "-p", "--path", "-v", "--verbose", "-V", "--version"});
  cmdl.parse(argc, argv);

  const bool isVersion = cmdl[{"-V", "--version"}];
  const bool isVerbose = cmdl[{"-v", "--verbose"}];
  const bool isHelp = cmdl[{"-h", "--help"}];
  const std::string path = cmdl("p").str().empty() ? cmdl("path").str() : cmdl("p").str();

  if (isVersion)
  {
    showVersion();
    return 0;
  }

  if (isHelp)
  {
    TerminalHandle terminal;
    terminal->showHelp({
      {"-h, --help", {"Show this help."}},
      {"-p, --path", {"Path in which a \".potcreator.json\" file exists", "to use it for generating the *.pot file."}},
      {"-v, --verbose", {"Show verbose output while running."}},
      {"-V, --version", {"Show current version."}}
    });
    return 0;
  }

  if (path.empty())
  {
    std::cout << "empty path" << std::endl;
    return 1;
  }

  pimpl->cfg = ps::potcreator::getConfigFromFile(path);
  pimpl->cfg.isVerbose = isVerbose;

  Progress modulesProgress;
  modulesProgress.id = TERMINAL_MODULES_ID;
  modulesProgress.displayName = "Modules";
  modulesProgress.max = pimpl->cfg.modules.size();
  modulesProgress.current = pimpl->modulesDone;

  {
    TerminalHandle terminal;
    terminal->addProgress(modulesProgress);
  }

  pimpl->fetchTranslationsForModule<GDScriptModule>();
  pimpl->fetchTranslationsForModule<GDSceneModule>();
  pimpl->fetchTranslationsForModule<SqliteModule>();
  pimpl->fetchTranslationsForModule<JsonModule>();

  pimpl->translations = unifyOutput(pimpl->translations);

  {
    TerminalHandle terminal;
    terminal->cleanLine();
  }

  genPot(pimpl->cfg, pimpl->translations);

  std::filesystem::path outputFile = pimpl->cfg.basePath / pimpl->cfg.outputPath.relative_path();
  outputFile.make_preferred();

  std::cout << outputFile << " generated" << std::endl;

  return 0;
}

} // namespace potcreator
} // namespace ps
