#include <vector>
#include "DafLogger.h"
#include "parsing/ArgHandler.h"
#include "parsing/Parser.h"

using std::vector;

void printFilesForParsing(vector<FileForParsing>& ffps) {
    logDaf(NOTE, "Files for parsing:");
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::cout << i << ": " << ffps[i].inputName<< ": " << ffps[i].inputFile << " -> " << ffps[i].outputFile << std::endl;
    }
}

void doItAll(vector<FileForParsing>& ffps) {
    vector<std::unique_ptr<ParsedFile>> parsedFiles;
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::unique_ptr<ParsedFile> parsedFile = parseFileSyntax(ffps[i], ffps[i].fullParse);
        ffps[i].parsedFile = parsedFile.get();
        //If fullParse, find imports. Add'em if not already added. Have the imports point to the file for parsing
        parsedFiles.push_back(std::move(parsedFile));
    }
}

int main(int argc, const char** argv) {
    vector<FileForParsing> ffps = parseParameters(argc, argv);
    printFilesForParsing(ffps);
    terminateIfErrors(); //File duplicates
    doItAll(ffps);
}
