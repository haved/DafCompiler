#include "DafLogger.hpp"
#include "FileController.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/NameScopeParser.hpp"
#include "CodegenLLVM.hpp"

constexpr bool debug =
#ifdef DAF_DEBUG
	true;
#else
	false;
#endif
constexpr bool printFilesGiven = debug;
constexpr bool dumpNameScope = debug&&false;
constexpr bool dumpLLVMModule = debug;
constexpr bool tellMeAboutGracefullShutdown = debug;

using std::vector;

int main(int argc, const char** argv) {
	FileRegistry files = parseCommandArguments(argc, argv);
	if constexpr(printFilesGiven)
		files.printFiles();
	terminateIfErrors(); //File duplicates are errors

	for(int i = 0; i < files.getFileCount(); i++) {
		Lexer lexer(files.getFileReference(i));
		parseFileAsNameScope(lexer, &files.getFileAt(i)->m_nameScope);
		if constexpr(dumpNameScope) {
			files.getFileAt(i)->m_nameScope->printSignature();
			std::cout << std::endl;
		}
	}
	terminateIfErrors();

	DependencyMap dependencyGraph;
	NamespaceStack ns_stack;
	//This is where we make files importable though the namespace stack
	for(int i = 0; i < files.getFileCount(); i++) {
		NameScope& scope = *files.getFileAt(i)->m_nameScope; //An optional
		scope.makeConcrete(ns_stack, dependencyGraph); //Recursive
	}
	terminateIfErrors();

	assert(!dependencyGraph.anyLostCauses());

	if(dependencyGraph.complainAboutLoops())
		terminateIfErrors(), assert(false);

    CodegenLLVM codegen("The global daf LLVM module");
	doCodegen(codegen, files);
	terminateIfErrors();

	if constexpr(dumpLLVMModule)
		codegen.Module().dump();

	outputCodegenToFile(codegen, files.getOutput());
	llvm::llvm_shutdown(); //There will still be objects on the heap after this ;(
	terminateIfErrors();

	if constexpr(tellMeAboutGracefullShutdown)
		puts("DafCompiler: Shutdown gracefully");

	return 0;
}
