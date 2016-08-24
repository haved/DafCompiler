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
    bool recursive=false;
    std::string output;
};

struct FileForParsing {
    std::string inputFile;
    std::string outputFile;
    bool recursive;
    FileForParsing(std::string inputFile, std::string outputFile, bool recursive) {
        this->inputFile = inputFile;
        this->outputFile = outputFile;
        this->recursive = recursive;
    }
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
            if (output.output.size()!=0)
                logDafC(WARNING) << "Overriding '" << output.output << "' as output" << std::endl;
            output.output.assign(argv[i]);
        } else if(strcmp(argv[i], "-r")==0) {
            if(output.recursive)
                logDaf(WARNING, "Duplicate option '-r'");
            output.recursive = true;
        } else if(strcmp(argv[i], "--help")==0) {
            printHelpPage();
            std::exit(0);
        } else {
            output.inputFiles.push_back(std::string(argv[i]));
        }
    }

    output.inputFiles.shrink_to_fit();
    output.searchDirs.shrink_to_fit();
    return output;
}

void assureCommandInput(CommandInput& input) {
    if(input.searchDirs.size()==0)
        input.searchDirs.push_back(".");
    if(input.output.size()==0)
        input.output.assign("./");
}

vector<FileForParsing> handleCommandInput(CommandInput& input) {
    if(input.inputFiles.size() == 0)
        logDaf(FATAL_ERROR, "No input files");
    //Check if output directory or file
    vector<FileForParsing> ffps(input.inputFiles.size());
    bool failed = false;
    for(int i = 0; i < input.inputFiles.size(); i++) {
        ffps.push_back(FileForParsing(input.inputFiles))
    }
    return ffp;
}

int main(int argc, char** argv) {
    CommandInput input = handleArgs(argc, argv);
    assureDefaultInput(input);
    handleCommandInput(input);
}
