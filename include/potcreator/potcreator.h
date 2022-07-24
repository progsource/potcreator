#ifndef PS_POTCREATOR_H
#define PS_POTCREATOR_H

#include <string>
#include <memory>

namespace ps {
namespace potcreator {

const std::string VERSION = "0.3.0";

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
