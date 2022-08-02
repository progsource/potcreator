#include "potcreator/potcreator.h"

#include <iostream>

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

} // empty namespace

struct PotCreator::Pimpl
{
  rxterm::VirtualTerminal vt;

  unsigned getTerminalWidth()
  {
    unsigned w = rxterm::VirtualTerminal::width();
    if (!w) w = 80;
    return w;
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
    // TODO: display red error
    return 1; // TODO: use enum value for return value
  }

  auto cfg = ps::potcreator::getConfigFromFile(path);
  cfg.isVerbose = isVerbose;

  std::vector<Output> translations;

  uint32_t modulesDone = 0;

  {
    TerminalHandle terminal;

    Progress modulesProgress;
    modulesProgress.id = 1;
    modulesProgress.displayName = "Modules";
    modulesProgress.max = cfg.modules.size();
    modulesProgress.current = modulesDone;
    terminal->addProgress(modulesProgress);
  }

  if (cfg.hasModule(GDScriptModule::MODULE_NAME))
  {
    GDScriptModule gdscriptModule;
    std::vector<Output> gdscriptTranslations = gdscriptModule.getTranslations(cfg);
    mergeOutput(translations, gdscriptTranslations);
    modulesDone++;

    {
      TerminalHandle terminal;

      terminal->updateProgress(1, modulesDone);
    }
  }

  if (cfg.hasModule(GDSceneModule::MODULE_NAME))
  {
    GDSceneModule gdsceneModule;
    std::vector<Output> gdsceneTranslations = gdsceneModule.getTranslations(cfg);
    mergeOutput(translations, gdsceneTranslations);
    modulesDone++;

    {
      TerminalHandle terminal;

      terminal->updateProgress(1, modulesDone);
    }
  }

  genPot(cfg, translations);

  std::cout << cfg.basePath / cfg.outputPath.relative_path() << " generated" << std::endl;

  return 0;
}


} // namespace potcreator
} // namespace ps
