#include <iostream>
#include <vector>
#include <string>

using std::vector;

struct CommandInput {
    std::vector<std::string> searchDirs;
    std::vector<std::string> inputFiles;
    bool recursive;
    std::string output;
};

CommandInput handleArgs(int argc, char** argv) {
    CommandInput output;

    return output;
}

int main(int argc, char** argv) {
    std::cout << "The daf compiler" << std::endl;
    CommandInput input = handleArgs(argc, argv);
}
