#ifndef PS_POTCREATOR_GDSCENE_H
#define PS_POTCREATOR_GDSCENE_H

#include <string>
#include <vector>

#include "potcreator/potcreator_module.h"

namespace ps {
namespace potcreator {

class GDSceneModule : public Module
{
public:
  static const std::string MODULE_NAME;
  static const uint32_t TERMINAL_ID;

  GDSceneModule() = default;
  virtual ~GDSceneModule();

  std::vector<Output> getTranslations(const Config& cfg) const override;
};

} // namespace potcreator
} // namespace ps

#endif // PS_POTCREATOR_GDSCENE_H
