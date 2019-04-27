#pragma once

#include "parsing/ast/NameScope.hpp"
#include "RegisteredFile.hpp"
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

namespace fs = boost::filesystem;
using boost::optional;

struct RegisteredFileInternal {
	std::string m_inputName; //input name
	fs::path m_inputFile; //source directory + input name
	bool m_sourceIncluded;
	optional<NameScope> m_nameScope;
	RegisteredFileInternal(std::string&& inputName, fs::path&& inputFile, bool sourceIncluded);
	RegisteredFileInternal(RegisteredFileInternal&& other) = default;
	RegisteredFileInternal(RegisteredFileInternal& other) = delete;
	void printOneliner();
};

struct SourcePath {
	bool sourceIncluded; //Means we put the source in the output object file
	fs::path path;
	SourcePath(bool sourceIncluded, fs::path&& path);
};

//Handles the entire compile job
class FileRegistry {
private:
	std::vector<RegisteredFileInternal> m_registeredFiles;
	std::vector<SourcePath> m_sourcePaths;
	bool m_outputFileSet;
	fs::path m_outputFile;
	optional<fs::path> m_linkFileOutput;
	std::vector<std::string> m_linkfileStatements;
public:
	FileRegistry();
	bool tryAddPath(fs::path&& path, bool sourceIncluded);
	bool tryAddFile(std::string&& path);
	bool setOutput(fs::path&& path);
	fs::path& getOutput();
	inline int getSourcePathCount() { return m_sourcePaths.size(); }
	inline int getFileCount() { return m_registeredFiles.size(); }
	inline RegisteredFileInternal* getFileAt(uint index) { return &m_registeredFiles[index]; }
	inline RegisteredFile getFileReference(uint index) { assert(index < m_registeredFiles.size()); return RegisteredFile(this, index); }
	void printFiles();
};

FileRegistry parseCommandArguments(int argc, const char** argv);
