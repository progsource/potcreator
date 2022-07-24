#ifndef PS_POTCREATOR_MODULE_H
#define PS_POTCREATOR_MODULE_H

#include <string>
#include <vector>

#include "potcreator/potcreator_output.h"

namespace ps {
namespace potcreator {

struct Config;

class Module
{
public:
  Module() = default;
  virtual ~Module() = default;

  virtual std::vector<Output> getTranslations(const Config& cfg) const = 0;
};

} // namespace potcreator
} // namespace ps

#endif // PS_POTCREATOR_MODULE_H
