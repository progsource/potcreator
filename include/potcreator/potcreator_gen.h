#ifndef PS_POTCREATOR_GEN_H
#define PS_POTCREATOR_GEN_H

#include <vector>

namespace ps {

namespace potcreator {

struct Config;
struct Output;

void genPot(const Config& cfg, const std::vector<Output>& translations);

} // namespace potcreator

} // namespace ps

#endif // PS_POTCREATOR_GEN_H
