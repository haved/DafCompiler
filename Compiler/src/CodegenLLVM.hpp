#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>

class CodegenLLVM {
private:
	llvm::LLVMContext m_context;
	llvm::IRBuilder<> m_builder;
	llvm::Module m_module;
public:
	CodegenLLVM(const std::string& moduleName);
	llvm::LLVMContext& Context();
	llvm::IRBuilder<>& Builder();
	llvm::Module& Module();
};

class FileRegistry;
void doCodegen(FileRegistry& files);
