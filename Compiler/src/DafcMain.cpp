#include <vector>
#include "DafLogger.hpp"
#include "parsing/lexing/ArgHandler.hpp"
#include "parsing/Parser.hpp"

using std::vector;

void printFilesForParsing(vector<FileForParsing>& ffps) {
	logDaf(NOTE) << "Files for parsing:" << std::endl;
	for(unsigned int i = 0; i < ffps.size(); i++) {
		std::cout << i << ": " << ffps[i].m_inputName<< ": " << ffps[i].m_inputFile << " -> " << ffps[i].m_outputFile << std::endl;
	}
}

void doItAll(vector<FileForParsing>& ffps) {
	for(auto it = ffps.begin(); it != ffps.end(); ++it)
		parseFileSyntax(*it);
}

int main(int argc, const char** argv) {
	vector<FileForParsing> ffps = parseParameters(argc, argv);
	printFilesForParsing(ffps);
	terminateIfErrors(); //File duplicates
	doItAll(ffps);
}
