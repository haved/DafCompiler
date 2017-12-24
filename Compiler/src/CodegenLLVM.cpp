#include "CodegenLLVM.hpp"
#include "FileController.hpp"
#include "parsing/ast/NameScope.hpp"
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>

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

void doCodegen(CodegenLLVM& codegen, FileRegistry& files) {
	for(int i = 0; i < files.getFileCount(); i++) {
		NameScope& scope = *files.getFileAt(i)->m_nameScope;
		scope.codegen(codegen);
	}
}

void outputCodegenToFile(CodegenLLVM& codegen, fs::path& outputFile) {
	//The following is taken directly from the Kaleidoscope tutorial

	llvm::Module& module = codegen.Module();

	auto targetTriple = llvm::sys::getDefaultTargetTriple();
#ifdef DAF_TARGET_ONLY_x86
	LLVMInitializeX86TargetInfo();
	LLVMInitializeX86Target();
	LLVMInitializeX86TargetMC();
	LLVMInitializeX86AsmParser();
	LLVMInitializeX86AsmPrinter();
#else
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();
#endif
	std::string errorStr;
	auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, errorStr);

	if(!Target) {
		logDaf(FATAL_ERROR) << errorStr << std::endl;
		terminateIfErrors();
	}

	auto CPU = "generic";
	auto features = "";

	llvm::TargetOptions opt;
	auto RM = llvm::Optional<llvm::Reloc::Model>();
	auto targetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);

	module.setDataLayout(targetMachine->createDataLayout());
	module.setTargetTriple(targetTriple);

	std::string Filename = outputFile.string();
	std::error_code EC;
	llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::F_None);

	if(EC) {
		logDaf(FATAL_ERROR) << "Could not open file("<<Filename<<"): " << EC.message() << std::endl;
		terminateIfErrors();
	}

	llvm::legacy::PassManager pass;
	auto fileType = llvm::TargetMachine::CGFT_ObjectFile;

	if(targetMachine->addPassesToEmitFile(pass, dest, fileType)) {
		logDaf(FATAL_ERROR) << "TargetMachine can't emit a file of this type, says LLVM" << std::endl;
		terminateIfErrors();
	}

	pass.run(module);
	dest.flush();
}

