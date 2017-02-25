#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <memory>
#include "parsing/ast/NameScope.hpp"

namespace fs = boost::filesystem;
using boost::optional;
using std::vector;
using std::unique_ptr;

struct FileForParsing {
    fs::path m_inputName;
    fs::path m_inputFile;
    fs::path m_canonicalInput;
    fs::path m_outputFile;
    bool m_outputFileSet;
    optional<NameScope> m_nameScope;
    FileForParsing(const fs::path& inputName, const fs::path& outputFile, bool outputFileSet);
};

vector<FileForParsing> parseParameters(int argc, const char** argv);
bool removeDuplicates(vector<FileForParsing>& ffps, bool log);
