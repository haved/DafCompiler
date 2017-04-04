#include "DafLogger.hpp"
#include "FileController.hpp"
#include "parsing/NameScopeParser.hpp"
#include "parsing/lexing/Lexer.hpp"

using std::vector;

int main(int argc, const char** argv) {
	FileRegistry files = parseCommandArguments(argc, argv);
	files.printFiles();
	terminateIfErrors(); //File duplicates

	for(int i = 0; i < files.getFileCount(); i++) {
		Lexer lexer(files.getFileReference(i));
		parseFileAsNameScope(lexer, &files.getFileAt(i)->m_nameScope);
		files.getFileAt(i)->m_nameScope->printSignature();
		std::cout << std::endl;
	}
	terminateIfErrors();

	//TODO: Reference evaluation and type inferring

	

	//TODO: Code gen
}
