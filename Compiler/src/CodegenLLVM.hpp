#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
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
