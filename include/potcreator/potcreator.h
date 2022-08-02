#ifndef PS_POTCREATOR_H
#define PS_POTCREATOR_H

#include <string>
#include <memory>

namespace ps {
namespace potcreator {

#define STR_(x) #x
#define STR(x) STR_(x)

#ifndef POTCREATOR_VERSION
#define POTCREATOR_VERSION "unknown"
#endif

const std::string VERSION = STR(POTCREATOR_VERSION);

class PotCreator
{
public:
  PotCreator();
  ~PotCreator();

  int run(int argc, char** argv);

private:
  struct Pimpl;
  std::unique_ptr<Pimpl> pimpl;
};

} // namespace potcreator
} // namespace ps

#endif // PS_POTCREATOR_H
