#include "potcreator/potcreator_helper.h"

#include <regex>

namespace ps {

namespace potcreator {

std::string
getDisplayPath(std::filesystem::path basePath, std::filesystem::path path)
{
  static const std::regex replaceBackslash("\\\\");

  std::string pathString = path.u8string().substr(basePath.u8string().size());
  pathString = std::regex_replace(pathString, replaceBackslash, "/");

  if (pathString[0] == '/')
  {
    pathString = pathString.substr(1);
  }

  return pathString;
}

} // namespace potcreator

} // namespace ps
