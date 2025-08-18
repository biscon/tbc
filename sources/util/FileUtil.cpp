//
// Created by bison on 18-08-25.
//

#include "FileUtil.h"

// baseFile: file that contains the reference (e.g. your JSON file)
// relativeRef: the relative path inside the file (e.g. "../image.png")
fs::path ResolveRelativeToCwd(const fs::path& baseFile, const fs::path& relativeRef) {
    fs::path baseDir = baseFile.parent_path();
    fs::path combined = baseDir / relativeRef;

    // Normalize "../" and "./"
    fs::path normalized = combined.lexically_normal();

    // Get current working dir
    fs::path cwd = fs::current_path();

    // Make relative to cwd if possible
    return fs::relative(normalized, cwd);
}
