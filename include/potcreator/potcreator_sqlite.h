#ifndef PS_POTCREATOR_SQLITE_H
#define PS_POTCREATOR_SQLITE_H

#include <string>
#include <vector>

#include "potcreator/potcreator_module.h"

namespace ps {

namespace potcreator {

class SqliteModule : public Module
{
public:
  static const std::string MODULE_NAME;
  static const uint32_t TERMINAL_ID;

  SqliteModule() = default;
  virtual ~SqliteModule();

  std::vector<Output> getTranslations(const Config& cfg) const override;
};

} // namespace potcreator

} // namespace ps

#endif // PS_POTCREATOR_SQLITE_H
