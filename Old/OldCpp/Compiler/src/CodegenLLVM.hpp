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

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class FunctionExpression;

class CodegenLLVM {
private:
	llvm::LLVMContext m_context;
	llvm::IRBuilder<> m_builder;
	llvm::Module m_module;
	FunctionExpression* m_funcExpr;
public:
	CodegenLLVM(const std::string& moduleName);
	CodegenLLVM(const CodegenLLVM& other)=delete;
	CodegenLLVM& operator=(const CodegenLLVM& other)=delete;
	llvm::LLVMContext& Context();
	llvm::IRBuilder<>& Builder();
	llvm::Module& Module();
	FunctionExpression* getFunctionExpression();
	FunctionExpression* pushFunctionExpression(FunctionExpression* funcExpr);
	void popFunctionExpression(FunctionExpression* funcExpr);
};

class FileRegistry;
void doCodegen(CodegenLLVM& codegen, FileRegistry& files);
void outputCodegenToFile(CodegenLLVM& codegen, fs::path& outputFile);
