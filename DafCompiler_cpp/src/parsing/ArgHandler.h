#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <memory>
#include "parsing/Parser.h"

namespace fs = boost::filesystem;
using std::vector;

struct FileForParsing {
    fs::path inputName;
    fs::path inputFile;
    fs::path canonicalInput;
    fs::path outputFile;
    bool outputFileSet;
    bool recursive;
    bool fullParse; //If there is to be an output
    ParsedFile* parsedFile;
    FileForParsing(const fs::path& inputName, const fs::path& outputFile, bool outputFileSet, bool recursive, bool fullParse);
};

vector<FileForParsing> parseParameters(int argc, const char** argv);
bool removeDuplicates(vector<FileForParsing>& ffps, bool log);
