#include <vector>
#include "DafLogger.h"
#include "ArgHandler.h"

using std::vector;

void printFilesForParsing(vector<FileForParsing>& ffps) {
    logDaf(NOTE, "Files for parsing:");
    for(unsigned int i = 0; i < ffps.size(); i++) {
        std::cout << ffps[i].ID << ": " << ffps[i].inputFile << " -> " << ffps[i].outputFile << std::endl;
    }
}

int main(int argc, const char** argv) {
    vector<FileForParsing> ffps = parseParameters(argc, argv);
    //Error on duplicates
    printFilesForParsing(ffps);
}
