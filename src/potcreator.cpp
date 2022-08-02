#include "potcreator/potcreator.h"

#include <iostream>
#include <filesystem>

#include "rxterm/terminal.hpp"
#include "rxterm/style.hpp"
#include "rxterm/image.hpp"
#include "rxterm/components/text.hpp"
#include "rxterm/components/stacklayout.hpp"
#include "rxterm/components/flowlayout.hpp"
#include "rxterm/components/progress.hpp"
#include "rxterm/components/maxwidth.hpp"

#include "argh.h"

#include "potcreator/potcreator_config.h"
#include "potcreator/potcreator_gdscript.h"
#include "potcreator/potcreator_gdscene.h"
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

void clear()
{
  std::cout << "\033[0m" << std::endl;
}

auto renderToTerm = [](auto const& vt, unsigned const w, rxterm::Component const& c) {
  return vt.flip(c.render(w).toString());
};

void showVersion()
{
  std::cout << "potcreator version " << VERSION << std::endl;
}

void showHelp(rxterm::VirtualTerminal & vt, unsigned const w)
{
  vt = renderToTerm(vt, w, rxterm::StackLayout<>{
    rxterm::Text({rxterm::Color::Black, rxterm::FontColor::White, rxterm::Font::Bold}, "potcreator"),
    rxterm::FlowLayout<>{
      rxterm::Text("\t"),
      rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Cyan, rxterm::Font::Bold}, "-h, --help"),
      rxterm::Text("\t\t"),
      rxterm::Text("Show this help."),
    },
    rxterm::FlowLayout<>{
      rxterm::Text("\t"),
      rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Cyan, rxterm::Font::Bold}, "-p, --path"),
      rxterm::Text("\t\t"),
      rxterm::StackLayout<>{
        rxterm::Text("Path in which a \".potcreator.json\" file exists"),
        rxterm::Text("to use it for generating the *.pot file."),
      },
    },
    rxterm::FlowLayout<>{
      rxterm::Text("\t"),
      rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Cyan, rxterm::Font::Bold}, "-v, --verbose"),
      rxterm::Text("\t\t"),
      rxterm::Text("Show verbose output while running."),
    },
    rxterm::FlowLayout<>{
      rxterm::Text("\t"),
      rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Cyan, rxterm::Font::Bold}, "-V, --version"),
      rxterm::Text("\t\t"),
      rxterm::Text("Show current version."),
    },
  });

  clear();
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
  rxterm::VirtualTerminal vt;
  uint32_t modulesDone = 0;
  std::vector<Output> translations;

  unsigned getTerminalWidth()
  {
    unsigned w = rxterm::VirtualTerminal::width();
    if (!w) w = 80;
    return w;
  }

  template <typename T>
  void fetchTranslationsForModule()
  {
    if (cfg.hasModule(T::MODULE_NAME))
    {
      std::vector<Output> moduleranslations = getTranslationsFromModule<T>(cfg);
      mergeOutput(translations, moduleranslations);
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

  const bool isVersion = cmdl[{ "-V", "--version" }];
  const bool isVerbose = cmdl[{ "-v", "--verbose" }];
  const bool isHelp = cmdl[{ "-h", "--help" }];
  const std::string path = cmdl("p").str().empty() ? cmdl("path").str() : cmdl("p").str();

  if (isVersion)
  {
    showVersion();
    return 0;
  }

  if (isHelp)
  {
    showHelp(this->pimpl->vt, this->pimpl->getTerminalWidth());
    return 0;
  }

  if (path.empty())
  {
    std::cout << "empty path" << std::endl;
    return 1; // TODO: use enum value for return value
  }

  pimpl->cfg = ps::potcreator::getConfigFromFile(path);
  pimpl->cfg.isVerbose = isVerbose;

  {
    TerminalHandle terminal;

    Progress modulesProgress;
    modulesProgress.id = TERMINAL_MODULES_ID;
    modulesProgress.displayName = "Modules";
    modulesProgress.max = pimpl->cfg.modules.size();
    modulesProgress.current = pimpl->modulesDone;
    terminal->addProgress(modulesProgress);
  }

  pimpl->fetchTranslationsForModule<GDScriptModule>();
  pimpl->fetchTranslationsForModule<GDSceneModule>();

  genPot(pimpl->cfg, pimpl->translations);

  std::filesystem::path outputFile = pimpl->cfg.basePath / pimpl->cfg.outputPath.relative_path();
  outputFile.make_preferred();

  std::cout << outputFile << " generated" << std::endl;

  return 0;
}


} // namespace potcreator
} // namespace ps
