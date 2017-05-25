#include "DafLogger.hpp"
#include "FileController.hpp"
#include "parsing/NameScopeParser.hpp"
#include "parsing/lexing/Lexer.hpp"

#define DEBUG

using std::vector;

int main(int argc, const char** argv) {
	FileRegistry files = parseCommandArguments(argc, argv);
#ifdef DEBUG
	files.printFiles();
#endif
	terminateIfErrors(); //File duplicates are errors

	for(int i = 0; i < files.getFileCount(); i++) {
		Lexer lexer(files.getFileReference(i));
		parseFileAsNameScope(lexer, &files.getFileAt(i)->m_nameScope);
#ifdef DEBUG
		files.getFileAt(i)->m_nameScope->printSignature();
		std::cout << std::endl;
#endif
	}
	terminateIfErrors();

	//TODO: Reference evaluation and type inferring

	/*
	//We start with the first file
	//NOTE: This can be done while adding the definitions
	for(int i = 0; i < files.getFileCount(); i++) {
		NameScope& scope = *files.getFileAt(i)->m_nameScope;
		scope.makeDefinitionMap(); //Recursive
	}
	terminateIfErrors(); //Definitions given the same name
	//We must make all name scopes' maps before we make stuff concrete
	*/

	for(int i = 0; i < files.getFileCount(); i++) {
		NameScope& scope = *files.getFileAt(i)->m_nameScope; //An optional
		scope.makeEverythingConcrete(); //Recursive
	}
	terminateIfErrors(); //This step could definitely log errors

	//TODO: Code gen
}
