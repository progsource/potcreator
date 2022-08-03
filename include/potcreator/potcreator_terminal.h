#ifndef PS_POTCREATOR_TERMINAL_H
#define PS_POTCREATOR_TERMINAL_H

#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <memory>

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

  void showHelp(const std::map<std::string, std::vector<std::string>>& parameters);

  void addProgress(const Progress& progress);
  void updateProgress(uint32_t id, uint32_t current);
  void incrementProgress(uint32_t id);

private:
  struct Impl;
  std::unique_ptr<Impl> impl;

  static std::mutex instanceMutex;
  static Terminal terminal;

  Terminal();
  ~Terminal();

  void displayProgress(uint32_t id);
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
