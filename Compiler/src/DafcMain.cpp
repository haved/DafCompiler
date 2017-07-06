#include "DafLogger.hpp"
#include "FileController.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/NameScopeParser.hpp"
#include "CodegenLLVM.hpp"

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

	NamespaceStack ns_stack;
	//This is where we make files importable though the namespace stack
	for(int i = 0; i < files.getFileCount(); i++) {
		NameScope& scope = *files.getFileAt(i)->m_nameScope; //An optional
		scope.makeConcrete(ns_stack); //Recursive
	}
	terminateIfErrors();

	ns_stack.resolveDotOperators();

	terminateIfErrors();

    doCodegen(files);

	terminateIfErrors();

	return 0;
}
