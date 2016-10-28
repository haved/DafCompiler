#include <vector>
#include "DafLogger.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/Parser.hpp"

using std::vector;

void printFilesForParsing(vector<FileForParsing>& ffps) {
    logDaf(NOTE, "Files for parsing:");
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::cout << i << ": " << ffps[i].m_inputName<< ": " << ffps[i].m_inputFile << " -> " << ffps[i].m_outputFile << std::endl;
    }
}

void doItAll(vector<FileForParsing>& ffps) {
    vector<ParsedFile*> parsedFiles; //Just pointers to all the parsed files. Maybe unneccicary, because every file will have a m_parsedFile
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::unique_ptr<ParsedFile> parsedFile = parseFileSyntax(ffps[i]);
        parsedFiles.push_back(parsedFile.get()); //Changing the owner doesn't change the pointer
        ffps[i].m_parsedFile=std::move(parsedFile);
    }
}

int main(int argc, const char** argv) {
    vector<FileForParsing> ffps = parseParameters(argc, argv);
    printFilesForParsing(ffps);
    terminateIfErrors(); //File duplicates
    doItAll(ffps);
}
