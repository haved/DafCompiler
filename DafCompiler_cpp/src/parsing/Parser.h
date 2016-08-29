#pragma once

#include <memory>
#include <boost/filesystem.hpp>

namespace fs=boost::filesystem;

struct ParsedFile {
    bool fullyParsed;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const fs::path& destFile, bool fullParse);
