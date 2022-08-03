#ifndef PS_POTCREATOR_HELPER_H
#define PS_POTCREATOR_HELPER_H

#include <filesystem>
#include <string>

namespace ps {

namespace potcreator {

std::string getDisplayPath(std::filesystem::path basePath, std::filesystem::path path);

} // namespace potcreator

} // namespace ps

#endif // PS_POTCREATOR_HELPER_H
