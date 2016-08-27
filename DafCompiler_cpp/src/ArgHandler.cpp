/*#include <iostream>
#include <cstring>
#include <cstdlib>*/
#include "ArgHandler.h"
#include "DafLogger.h"

using std::vector;

struct CommandInput {
    vector<fs::path> searchDirs;
    vector<std::string> inputFiles;
    bool recursive=false;
    std::string output;
};

unsigned int FileForParsingNextId = 0;
FileForParsing::FileForParsing(const fs::path& inputFile, const fs::path& outputFile, bool outputFileSet, bool recursive, bool fullParse) {
    this->inputFile = inputFile;
    this->outputFile = outputFile;
    this->outputFileSet = outputFileSet;
    this->recursive = recursive;
    this->fullParse = fullParse;
    this->ID = FileForParsingNextId++; //Give a unique ID (overlap not happening anytime soon)
}

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

CommandInput handleArgs(int argc, const char** argv) {
    CommandInput output;

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--path")==0) {
            i++;
            if(i>=argc)
                logDaf(FATAL_ERROR, "Expected a search directory after '--path'");
            fs::path searchDir(argv[i]);
            if(!fs::is_directory(searchDir)) {
                logDafC(FATAL_ERROR) << "The search path '" << searchDir << "' doesn't exist" << std::endl;
                terminateIfErrors();
            }
            output.searchDirs.push_back(searchDir);
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

bool isOutputDir(std::string& output) {
    return output[output.size()-1]=='/'; //We can assert that the length is more than 0
}

vector<FileForParsing> handleCommandInput(CommandInput& input) {
    if(input.inputFiles.size() == 0)
        logDaf(FATAL_ERROR, "No input files");
    bool outputDir = isOutputDir(input.output);
    if(!outputDir && (input.recursive || input.inputFiles.size() > 1))
        logDaf(FATAL_ERROR, "With multiple input files, the output must be a directory");
    vector<FileForParsing> ffps;
    fs::path oExtension("o");
    for(unsigned int i = 0; i < input.inputFiles.size(); i++) {
        fs::path inputFile(input.inputFiles[i]);
        ffps.push_back(FileForParsing(inputFile, fs::path(input.output), !outputDir, input.recursive, true));
    }
    return ffps;
}

bool tryMakeFilePathReal(FileForParsing& ffp, vector<fs::path> searchDirs) {
    const fs::path dafExt(".daf");
    const fs::path oExt(".o");

    bool done = false;
    std::string attempt(ffp.inputFile.string());
    bool tryWithDaf = ffp.inputFile.extension()!=dafExt;
    while(!done) {
        for(unsigned int i = 0; i < searchDirs.size(); i++) {
            fs::path fullAttempt = searchDirs[i]/fs::path(attempt);
            if(fs::is_regular(fullAttempt)) {
                ffp.inputFile = fullAttempt;
                if(!ffp.outputFileSet)
                    ffp.outputFile = ffp.outputFile/attempt;
                done = true;
                break;
            }
            if(tryWithDaf) {
                fullAttempt = fullAttempt.concat(dafExt.string());
                if(fs::is_regular(fullAttempt)) {
                    ffp.inputFile = fullAttempt;
                    if(!ffp.outputFileSet)
                        ffp.outputFile = ffp.outputFile/attempt;
                    done = true;
                    break;
                }
            }
        }
        if(!done) {
            size_t dotPos = attempt.find('.');
            if(dotPos == std::string::npos) { //We never found the file :'(
                break;
            }
            attempt[dotPos] = '/';
        }
    }

    if(!done) {
        return false;
    }

    //Set output file properly
    if(!ffp.outputFileSet) {
        ffp.outputFileSet = true;
        if(ffp.outputFile.extension()==dafExt) {
            ffp.outputFile.replace_extension(oExt);
        } else {
            ffp.outputFile+=oExt;
        }
    }
    return true;
}

//Looks for the input files in the search directories, and moves their path there
//Also changes . to / in input and output
void assureInputOutput(vector<FileForParsing>& ffps, vector<fs::path>& searchDirs) {
    for(unsigned int i = 0; i < ffps.size(); i++) {
        if(tryMakeFilePathReal(ffps[i], searchDirs))
            continue;
        logDafC(ERROR) << "Input file " << ffps[i].inputFile << " not in a search path" << std::endl;
    }
    terminateIfErrors();
}

vector<FileForParsing> parseParameters(int argc, const char** argv) {
    CommandInput input = handleArgs(argc, argv);
    assureCommandInput(input); //Does default stuff
    vector<FileForParsing> ffps = handleCommandInput(input);
    assureInputOutput(ffps, input.searchDirs);
    return ffps;
}
