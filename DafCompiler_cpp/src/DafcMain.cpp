#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include "DafLogger.h"

using std::vector;

struct CommandInput {
    vector<std::string> searchDirs;
    vector<std::string> inputFiles;
    bool recursive;
    std::string output;
};

void printHelpPage() {
    std::cout   << "  Help page for dafc:" << std::endl
                << "  Usage: dafc [options] inputFiles" << std::endl
                << std::endl
                << "  Options:" << std::endl
                << "    --path <search>     Adds a directory for searching for source files" << std::endl
                << "    -o <output>         Sets the output directory or file" << std::endl
                << "    -r                  Enables recursive parsing" << std::endl
                << std::endl;
}

CommandInput handleArgs(int argc, char** argv) {
    CommandInput output;

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--path")==0) {
            i++;
            if(i>=argc)
                logDaf(FATAL_ERROR, "Expected a search directory after '--path'");
            output.searchDirs.push_back(std::string(argv[i]));
        } else if(strcmp(argv[i], "-o")==0) {
            i++;
            if(i>=argc)
                logDaf(FATAL_ERROR, "Expected an output file or directory after '-o'");
            if (output.output.size()!=0) {
                logDafC(WARNING) << "Overriding '" << output.output << "' as output" << std::endl;
                fatalErrorExit();
            }
            output.output.assign(argv[i]);
        } else if(strcmp(argv[i], "-r")==0) {
            output.recursive = true;
        } else if(strcmp(argv[i], "--help")==0) {
            printHelpPage();
            std::exit(0);
        } else {
            output.inputFiles.push_back(std::string(argv[i]));
        }
    }

    return output;
}

int main(int argc, char** argv) {
    CommandInput input = handleArgs(argc, argv);
}
