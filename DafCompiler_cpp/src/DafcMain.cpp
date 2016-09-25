#include <vector>
#include "DafLogger.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/Parser.hpp"

using std::vector;

void printFilesForParsing(vector<FileForParsing>& ffps) {
    logDaf(NOTE, "Files for parsing:");
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::cout << i << ": " << ffps[i].inputName<< ": " << ffps[i].inputFile << " -> " << ffps[i].outputFile << std::endl;
    }
}

void doItAll(vector<FileForParsing>& ffps) {
    vector<ParsedFile*> parsedFiles;
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::unique_ptr<ParsedFile> parsedFile = parseFileSyntax(ffps[i], ffps[i].fullParse);
        parsedFiles.push_back(parsedFile.get());
        //If fullParse, find imports. Add'em if not already added. Have the imports point to the file for parsing
        ffps[i].parsedFile=std::move(parsedFile);
    }
}

int main(int argc, const char** argv) {
    vector<FileForParsing> ffps = parseParameters(argc, argv);
    printFilesForParsing(ffps);
    terminateIfErrors(); //File duplicates
    doItAll(ffps);
}
