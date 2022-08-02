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

} // namespace potcreator
} // namespace ps
