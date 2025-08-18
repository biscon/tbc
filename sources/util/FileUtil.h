//
// Created by bison on 18-08-25.
//

#ifndef SANDBOX_FILEUTIL_H
#define SANDBOX_FILEUTIL_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

fs::path ResolveRelativeToCwd(const fs::path& baseFile, const fs::path& relativeRef);

#endif //SANDBOX_FILEUTIL_H
