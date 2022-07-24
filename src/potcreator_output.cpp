#include "potcreator/potcreator_output.h"

#include <algorithm>

namespace ps {
namespace potcreator {

namespace {

void makeUniqueValueVector(std::vector<std::string>& v)
{
  std::sort(v.begin(), v.end());
  auto it = std::unique(v.begin(), v.end());
  v.resize(std::distance(v.begin(), it));
}

} // empty namespace

void addOutput(std::vector<Output>& out, Output o)
{
  const auto it = std::find_if(out.begin(), out.end(), [search = o.key](const Output& output) {
    return search == output.key;
  });

  if (it == out.end())
  {
    out.push_back(o);
  }
  else
  {
    (*it).src.insert((*it).src.end(), o.src.begin(), o.src.end());
    (*it).extract.insert((*it).extract.end(), o.extract.begin(), o.extract.end());

    makeUniqueValueVector((*it).src);
    makeUniqueValueVector((*it).extract);
  }
}

void mergeOutput(std::vector<Output>& a, const std::vector<Output>& b)
{
  for (const auto& out : b)
  {
    addOutput(a, out);
  }

  std::sort(a.begin(), a.end(), [](const Output& a, const Output& b) -> bool { return a.key < b.key; });
}

} // namespace potcreator
} // namespace ps
