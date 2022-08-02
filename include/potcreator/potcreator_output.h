#ifndef PS_POTCREATOR_OUTPUT_H
#define PS_POTCREATOR_OUTPUT_H

#include <string>
#include <vector>

namespace ps {
namespace potcreator {

struct Output
{
  std::string key; // the actual key
  std::vector<std::string> src; // files from which the key comes from
  std::vector<std::string> extract; // other comments
};

void addOutput(std::vector<Output>& out, Output o);

void mergeOutput(std::vector<Output>& a, const std::vector<Output>& b);

} // namespace potcreator
} // namespace ps

#endif // PS_POTCREATOR_OUTPUT_H