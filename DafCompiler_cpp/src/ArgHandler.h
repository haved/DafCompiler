#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
using std::vector;

struct FileForParsing {
    fs::path inputFile;
    fs::path outputFile;
    bool outputFileSet;
    bool recursive;
    bool fullParse; //If there is to be an output
    unsigned int ID;
    FileForParsing(const fs::path& inputFile, const fs::path& outputFile, bool outputFileSet, bool recursive, bool fullParse);
};

vector<FileForParsing> parseParameters(int argc, const char** argv);
bool removeDuplicates(vector<FileForParsing>& ffps, bool log);
