#ifndef PS_POTCREATOR_TERMINAL_H
#define PS_POTCREATOR_TERMINAL_H

#include <string>
#include <map>
#include <mutex>

#include <iostream>

namespace ps {
namespace potcreator {

struct Progress
{
  uint32_t id;
  std::string displayName;
  uint32_t max;
  uint32_t current;
};

class Terminal
{
public:
  // for usage in thread world - we take and give back
  static Terminal* takeInstance();
  static void returnInstance();

  void addProgress(const Progress& progress)
  {
    this->progress[progress.id] = progress;
    this->displayProgress(progress.id);
  }

  void updateProgress(uint32_t id, uint32_t current)
  {
    if (!this->progress.count(id))
    {
      return;
    }

    this->progress.at(id).current = current;
    this->displayProgress(id);
  }

  void incrementProgress(uint32_t id)
  {
    if (!this->progress.count(id))
    {
      return;
    }

    this->progress.at(id).current++;
    this->displayProgress(id);
  }

private:
  static std::mutex instanceMutex;
  static Terminal terminal;
  std::map<uint32_t, Progress> progress;

  Terminal() = default;

  void displayProgress(uint32_t id)
  {
    std::cout << this->progress.at(id).displayName << ": " <<
      this->progress.at(id).current << "/" << this->progress.at(id).max <<
      std::endl;
  }
};

/**
 * TerminalHandle
 *
 * To not have to remember how to take and put back the terminal instance, this
 * scoped handle should be used instad.
 */
class TerminalHandle
{
public:
  TerminalHandle()
    : instance(Terminal::takeInstance())
  {}

  ~TerminalHandle()
  {
    this->instance = nullptr;
    Terminal::returnInstance();
  }

  Terminal* operator->() {
    return instance;
  }

private:
  Terminal* instance;
};

} // namespace potcreator
} // namespace ps

#endif // PS_POTCREATOR_TERMINAL_H
