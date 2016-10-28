#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <memory>
#include "parsing/ast/ParsedFile.hpp"

namespace fs = boost::filesystem;
using std::vector;
using std::unique_ptr;

struct FileForParsing {
    fs::path m_inputName;
    fs::path m_inputFile;
    fs::path m_canonicalInput;
    fs::path m_outputFile;
    bool m_outputFileSet;
    unique_ptr<ParsedFile> m_parsedFile;
    FileForParsing(const fs::path& inputName, const fs::path& outputFile, bool outputFileSet);
};

vector<FileForParsing> parseParameters(int argc, const char** argv);
bool removeDuplicates(vector<FileForParsing>& ffps, bool log);
