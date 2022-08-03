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

void mergeOutput(std::vector<Output>& a, const std::vector<Output>& b)
{
  a.insert(a.end(), b.begin(), b.end());
}

std::vector<Output> unifyOutput(const std::vector<Output>& o)
{
  if (o.empty())
  {
    return o;
  }

  std::vector<Output> out;

  for (const auto& output : o)
  {
    const auto it = std::find_if(
      out.begin(),
      out.end(),
      [search = output.key](const Output& output)
      {
        return search == output.key;
      }
    );

    if (it == out.end())
    {
      out.push_back(output);
    }
    else
    {
      (*it).src.insert((*it).src.end(), output.src.begin(), output.src.end());
      (*it).extract.insert((*it).extract.end(), output.extract.begin(), output.extract.end());
    }
  }

  std::sort(out.begin(), out.end(), [](const Output& a, const Output& b) -> bool { return a.key < b.key; });

  for (auto& output : out)
  {
    makeUniqueValueVector(output.src);
    makeUniqueValueVector(output.extract);
  }

  return out;
}

} // namespace potcreator
} // namespace ps
