#include "potcreator/potcreator_terminal.h"

// #include "rxterm/terminal.hpp"
// #include "rxterm/style.hpp"
// #include "rxterm/image.hpp"
// #include "rxterm/components/text.hpp"
// #include "rxterm/components/stacklayout.hpp"
// #include "rxterm/components/flowlayout.hpp"
// #include "rxterm/components/progress.hpp"
// #include "rxterm/components/maxwidth.hpp"

namespace ps {
namespace potcreator {

std::mutex Terminal::instanceMutex{};
Terminal Terminal::terminal{};

Terminal* Terminal::takeInstance()
{
  Terminal::instanceMutex.lock();
  return &terminal;
}

void Terminal::returnInstance()
{
  Terminal::instanceMutex.unlock();
}

void Terminal::addProgress(const Progress& progress)
{
  this->progress[progress.id] = progress;
  this->displayProgress(progress.id);
}

void Terminal::updateProgress(uint32_t id, uint32_t current)
{
  if (!this->progress.count(id))
  {
    return;
  }

  this->progress.at(id).current = current;
  this->displayProgress(id);
}

void Terminal::incrementProgress(uint32_t id)
{
  if (!this->progress.count(id))
  {
    return;
  }

  this->progress.at(id).current++;
  this->displayProgress(id);
}

} // namespace potcreator
} // namespace ps
