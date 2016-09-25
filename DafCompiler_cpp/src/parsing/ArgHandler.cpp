#include "ArgHandler.hpp"
#include "DafLogger.hpp"

using std::vector;

struct CommandInput {
    vector<fs::path> searchDirs;
    vector<std::string> inputFiles;
    bool recursive=false;
    std::string output;
    CommandInput() : searchDirs(), inputFiles(), output(){}
};

//unsigned int FileForParsingNextId = 0;
FileForParsing::FileForParsing(const fs::path& inputName, const fs::path& outputFile, bool outputFileSet, bool recursive, bool fullParse)
  : m_inputName(inputName), m_inputFile(fs::path("")), m_canonicalInput(fs::path("")),
    m_outputFile(outputFile), m_outputFileSet(outputFileSet), m_recursive(recursive), m_fullParse(fullParse),
    m_parsedFile(boost::none) {}

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
                logDaf(FATAL_ERROR) << "The search path '" << searchDir << "' doesn't exist" << std::endl;
                terminateIfErrors();
            }
            output.searchDirs.push_back(searchDir);
        } else if(strcmp(argv[i], "-o")==0) {
            i++;
            if(i>=argc)
                logDaf(FATAL_ERROR, "Expected an output file or directory after '-o'");
            if (output.output.size()!=0)
                logDaf(WARNING) << "Overriding '" << output.output << "' as output" << std::endl;
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

bool isInputFile(const fs::path& path) {
    return fs::is_regular_file(path);
}

bool tryMakeFilePathReal(FileForParsing& ffp, vector<fs::path> searchDirs) {
    const fs::path dafExt(".daf");
    const fs::path oExt(".o");

    bool done = false;
    std::string attempt(ffp.m_inputName.string());
    bool tryWithDaf = ffp.m_inputName.extension()!=dafExt;
    while(!done) {
        for(unsigned int i = 0; i < searchDirs.size(); i++) {
            fs::path fullAttempt = searchDirs[i]/fs::path(attempt);
            if(isInputFile(fullAttempt)) {
                ffp.m_inputFile = fullAttempt;
                if(!ffp.m_outputFileSet)
                    ffp.m_outputFile = ffp.m_outputFile/attempt;
                done = true;
                break;
            }
            if(tryWithDaf) {
                fullAttempt = fullAttempt.concat(dafExt.string());
                if(isInputFile(fullAttempt)) {
                    ffp.m_inputFile = fullAttempt;
                    if(!ffp.m_outputFileSet)
                        ffp.m_outputFile = ffp.m_outputFile/attempt;
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
    if(!ffp.m_outputFileSet) {
        ffp.m_outputFileSet = true;
        if(ffp.m_outputFile.extension()==dafExt) {
            ffp.m_outputFile.replace_extension(oExt);
        } else {
            ffp.m_outputFile+=oExt;
        }
    }
    return true;
}

//Looks for the input files in the search directories, and moves their path there
//Also changes . to / in input and output
//Also saves the canonical input file path
void assureInputOutput(vector<FileForParsing>& ffps, vector<fs::path>& searchDirs) {
    for(unsigned int i = 0; i < ffps.size(); i++) {
        if(tryMakeFilePathReal(ffps[i], searchDirs))
            continue;
        logDaf(ERROR) << "Input file " << ffps[i].m_inputName << " not in a search path" << std::endl;
    }
    terminateIfErrors();
    for(unsigned int i = 0; i < ffps.size(); i++) {
        ffps[i].m_canonicalInput = fs::canonical(ffps[i].m_inputFile);
    }
}

bool removeDuplicates(vector<FileForParsing>& ffps, bool log) {
    bool removed = false;
    for(unsigned int i = 0; i < ffps.size(); i++) {
        for(unsigned int j = i+1; j < ffps.size(); j++) {
            if(ffps[i].m_canonicalInput==ffps[j].m_canonicalInput) {
                removed = true;
                if(log)
                    logDaf(ERROR) << "File input twice: " << ffps[i].m_inputFile << std::endl;
                ffps.erase(ffps.begin()+j);
                j--;
            }
        }
    }
    return removed;
}

vector<FileForParsing> parseParameters(int argc, const char** argv) {
    CommandInput input = handleArgs(argc, argv);
    assureCommandInput(input); //Does default stuff
    vector<FileForParsing> ffps = handleCommandInput(input);
    assureInputOutput(ffps, input.searchDirs);
    removeDuplicates(ffps, true);
    return ffps;
}
