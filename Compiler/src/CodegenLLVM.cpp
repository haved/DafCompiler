#include "CodegenLLVM.hpp"
#include "FileController.hpp"
#include "parsing/ast/NameScope.hpp"

CodegenLLVM::CodegenLLVM(const std::string& moduleName) : m_context(), m_builder(m_context), m_module(moduleName, m_context) {}

llvm::LLVMContext& CodegenLLVM::Context() {
	return m_context;
}

llvm::IRBuilder<>& CodegenLLVM::Builder() {
	return m_builder;
}

llvm::Module& CodegenLLVM::Module() {
	return m_module;
}


void doCodegen(FileRegistry& files) {
    CodegenLLVM codegen("The global daf LLVM module");

	for(int i = 0; i < files.getFileCount(); i++) {
		NameScope& scope = *files.getFileAt(i)->m_nameScope;
		scope.codegen(codegen);
	}

	codegen.Module().dump();
}
