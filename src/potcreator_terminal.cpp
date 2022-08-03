#include "potcreator/potcreator_terminal.h"

#include <iomanip>
#include <sstream>

#include "rxterm/components/flowlayout.hpp"
#include "rxterm/components/maxwidth.hpp"
#include "rxterm/components/progress.hpp"
#include "rxterm/components/stacklayout.hpp"
#include "rxterm/components/text.hpp"
#include "rxterm/image.hpp"
#include "rxterm/style.hpp"
#include "rxterm/terminal.hpp"

#include "potcreator/potcreator_helper.h"

namespace ps {

namespace potcreator {

namespace {

void
clear()
{
  std::cout << "\033[0m" << std::endl;
}

auto renderToTerm = [](auto const& vt, unsigned const w, rxterm::Component const& c)
                    {
                      return vt.flip(c.render(w).toString());
                    };

auto
getParametersInLayout(const std::map<std::string, std::vector<std::string>>& parameters)
{
  auto getDescriptionStackLayout = [](std::vector<std::string> descriptions)
                                   {
                                     std::vector<rxterm::Text> texts;
                                     for (const std::string& str : descriptions)
                                     {
                                       texts.push_back(rxterm::Text(str));
                                     }
                                     return rxterm::StackLayout<rxterm::Text>{texts};
                                   };

  std::vector<rxterm::Component> parameterLayouts;
  parameterLayouts.push_back(
    rxterm::Text(
      {rxterm::Color::Black, rxterm::FontColor::White, rxterm::Font::Bold},
      "potcreator"
      )
    );

  for (const auto& param : parameters)
  {
    parameterLayouts.push_back(
      rxterm::FlowLayout<>{
            rxterm::Text("\t"),
            rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Cyan, rxterm::Font::Bold}, param.first),
            rxterm::Text("\t\t"),
            getDescriptionStackLayout(param.second),
          }
      );
  }

  return rxterm::StackLayout<rxterm::Component>(parameterLayouts);
}

std::string
getPercentageText(float percentage)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << percentage;
  return stream.str() + "%";
}

} // empty namespace

struct Terminal::Impl
{
  rxterm::VirtualTerminal vt;
  std::map<uint32_t, Progress> progress;
  std::vector<std::string> infos;
  std::vector<std::string> errors;

  unsigned getTerminalWidth()
  {
    unsigned w = rxterm::VirtualTerminal::width();
    if (!w) {w = 80;}
    return w;
  }

  void displayProgress()
  {
    std::vector<rxterm::Component> outputs;

    for (const std::string& err : errors)
    {
      outputs.push_back(rxterm::Text({rxterm::Color::Black, rxterm::FontColor::BrightRed}, err));
    }

    for (const auto& [key, prog] : this->progress)
    {
      const float progressPercentage = prog.max == 0 ? 0 : 100.f / float(prog.max) * float(prog.current);

      outputs.push_back(rxterm::Text(prog.displayName + ":"));
      outputs.push_back(
        rxterm::FlowLayout<rxterm::Text>(
          rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Yellow}, prog.current),
          rxterm::Text(" / "),
          rxterm::Text({rxterm::Color::Black, rxterm::FontColor::Green}, prog.max),
          rxterm::Text(" -> "),
          rxterm::Text(
            {rxterm::Color::Black, rxterm::FontColor::Magenta},
            getPercentageText(progressPercentage)
            )
          )
        );
    }

    for (const std::string& info : infos)
    {
      outputs.push_back(rxterm::Text({rxterm::Color::Black, rxterm::FontColor::BrightYellow}, info));
    }

    rxterm::StackLayout<rxterm::Component> layout(outputs);

    unsigned w = this->getTerminalWidth();
    this->vt = renderToTerm(this->vt, w, layout);
  }
};

std::mutex Terminal::instanceMutex{};
Terminal Terminal::terminal{};

Terminal::Terminal()
  : impl(std::make_unique<Terminal::Impl>())
{}

Terminal::~Terminal() {}

Terminal*
Terminal::takeInstance()
{
  Terminal::instanceMutex.lock();
  return &terminal;
}

void
Terminal::returnInstance()
{
  Terminal::instanceMutex.unlock();
}

void
Terminal::showHelp(const std::map<std::string, std::vector<std::string>>& parameters)
{
  unsigned w = this->impl->getTerminalWidth();
  this->impl->vt = renderToTerm(this->impl->vt, w, getParametersInLayout(parameters));
  clear();
}

void
Terminal::addProgress(const Progress& progress)
{
  this->impl->progress[progress.id] = progress;
  this->impl->displayProgress();
}

void
Terminal::updateProgress(uint32_t id, uint32_t current)
{
  if (!this->impl->progress.count(id))
  {
    return;
  }

  this->impl->progress.at(id).current = current;
  this->impl->displayProgress();
}

void
Terminal::incrementProgress(uint32_t id)
{
  if (!this->impl->progress.count(id))
  {
    return;
  }

  this->impl->progress.at(id).current++;
  this->impl->displayProgress();
}

void
Terminal::addInfo(const std::string& info)
{
  this->impl->infos.push_back(info);
  this->impl->displayProgress();
}

void
Terminal::addError(const std::string& err)
{
  this->impl->errors.push_back(err);
  this->impl->displayProgress();
}

void
Terminal::cleanLine()
{
  clear();
}

void
Terminal::displayProgress(uint32_t id)
{
  std::cout << this->impl->progress.at(id).displayName << ": " <<
    this->impl->progress.at(id).current << "/" << this->impl->progress.at(id).max <<
    std::endl;
}

} // namespace potcreator

} // namespace ps
