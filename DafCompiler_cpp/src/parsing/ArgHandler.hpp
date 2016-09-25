#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <memory>
#include <boost/optional.hpp>
#include "parsing/ast/ParsedFile.hpp"

namespace fs = boost::filesystem;
using std::vector;
using std::unique_ptr;
using boost::optional;

struct FileForParsing {
    fs::path inputName;
    fs::path inputFile;
    fs::path canonicalInput;
    fs::path outputFile;
    optional<unique_ptr<ParsedFile>> parsedFile;
    bool outputFileSet;
    bool recursive;
    bool fullParse; //If there is to be an output
    FileForParsing(const fs::path& inputName, const fs::path& outputFile, bool outputFileSet, bool recursive, bool fullParse);
};

vector<FileForParsing> parseParameters(int argc, const char** argv);
bool removeDuplicates(vector<FileForParsing>& ffps, bool log);
