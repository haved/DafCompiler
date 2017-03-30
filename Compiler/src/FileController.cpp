#include "FileController.hpp"
#include "DafLogger.hpp"

#include <cstring>

#define STR_SAME(x, y) (strcmp(x, y) == 0)

void printDafcHelpPage() {
	std::cout << "The dafc help page\n\
Usage: dafc [options] <sourceFiles>\n\
Source files are relative to the added paths, and parsing happens recursivly from there\n\
Options:\n\
  -S <sourcePath>           Adds a search path for daf code that'll be included in the object file\n\
  -I <includePath>          Adds a search path for daf code that won't be in the object file (headers)\n\
  -o <outputBinary>         Sets the output binary location\n\
  -oL <output linkfile>     Sets the output path and name for the exported linkfile\n\
  -L <statement>            Includes the given statement in the link file\n\
  -h --help                 Prints this help screen\n";
}

FileRegistry parseCommandArguments(int argc, const char** argv) {
	FileRegistry registry;
	std::vector<std::string> files;
	for(int i = 1; i < argc; i++) {
		const char* arg = argv[i];
		if(arg[0] != '-') {
			files.emplace_back(arg);
		}
		else if(STR_SAME(arg, "-S") || STR_SAME(arg, "-I")) { //Add source path
			i++;
			if(i>=argc)
				logDaf(FATAL_ERROR) << "expected a search path after " << arg << std::endl;
			else
				registry.tryAddPath(argv[i], arg[1]=='S');
		}
		else if(STR_SAME(arg, "-o")) {
			i++;
			if(i>=argc)
				logDaf(FATAL_ERROR) << "expected an output binary file after -o" << std::endl;
			else
				registry.setOutput(argv[i]);
		}
		else if(STR_SAME(arg, "-h") || STR_SAME(arg, "--help")) {
			printDafcHelpPage();
			std::exit(0);
		}
		else {
			logDaf(FATAL_ERROR) << "unknown option: " << arg << std::endl;
		}

		terminateIfErrors();
	}

	if(files.size() == 0) {
		logDaf(FATAL_ERROR) << "no input files" << std::endl;
		terminateIfErrors();
	}

	if(registry.getSourcePathCount() == 0)
		registry.tryAddPath(".", true);

	for(auto it = files.begin(); it != files.end(); ++it) {
		registry.tryAddFile(std::move(*it)); //Doesn't matter that we move it
		terminateIfErrors();
	}
	return registry;
}

SourcePath::SourcePath(bool p_sourceIncluded, fs::path&& p_path) : sourceIncluded(p_sourceIncluded), path(std::move(p_path)) {}

FileRegistry::FileRegistry() : m_registeredFiles(), m_sourcePaths(), m_outputFile(), m_linkFileOutput(), m_linkfileStatements() {}

bool FileRegistry::tryAddPath(fs::path&& path, bool sourceIncluded) {
	if(!fs::is_directory(path))
		logDaf(FATAL_ERROR) << "source path " << path << " doesn't exist" << std::endl;
	else
		m_sourcePaths.emplace_back(sourceIncluded, std::move(path));

	return true; //TODO
}

bool FileRegistry::setOutput(fs::path&& path) {
	if(m_outputFile.size()!=0)
		logDaf(WARNING) << "setting output a second time, from " << m_outputFile << " to " << path << std::endl;
	m_outputFile = std::move(path);
	return false; //TODO
}

bool FileRegistry::tryAddFile(std::string&& inputName) {
	fs::path inputFile;

	bool lastDirSource = false;
	bool found = false;

	for(auto dir = m_sourcePaths.begin(); dir != m_sourcePaths.end(); ++dir) {
		lastDirSource = dir->sourceIncluded;
			inputFile = dir->path/inputName;
		if(is_regular_file(inputFile)) {
			found = true;
			break;
		}
		inputFile += ".daf";
		if(is_regular_file(inputFile)) {
			found = true;
			break;
		}
	}

	if(found) {
		for(auto prevFile = m_registeredFiles.begin(); prevFile != m_registeredFiles.end(); ++prevFile) {
			if(equivalent(prevFile->m_inputFile, inputFile)) {
				logDaf(FATAL_ERROR) << "file " << inputName << " already added" << std::endl;
				terminateIfErrors();
			}
		}
		m_registeredFiles.emplace_back(std::move(inputName), std::move(inputFile), lastDirSource);
		return true;
	}
	else {
		logDaf(FATAL_ERROR) << inputName << ": file not found" << std::endl;
		return false;
	}
}

void FileRegistry::printFiles() {
	std::cout << "Files for parsing:" << std::endl;
	for(auto file = m_registeredFiles.begin(); file != m_registeredFiles.end(); ++file) {
		file->printOneliner();
	}
}

RegisteredFileInternal::RegisteredFileInternal(std::string&& inputName, fs::path&& inputFile, bool sourceIncluded) : m_inputName(std::move(inputName)), m_inputFile(std::move(inputFile)), m_sourceIncluded(sourceIncluded), m_nameScope(boost::none) {}

void RegisteredFileInternal::printOneliner() {
	std::cout << m_inputName << "  at: " << m_inputFile << "   sourceIncluded:   " << (m_sourceIncluded?"yes":"no") << std::endl;
}

const RegisteredFileInternal& RegisteredFile::get() const {
	return *m_registry->getFileAt(m_fileIndex);
}

bool RegisteredFile::operator ==(const RegisteredFile& other) const {
	return m_fileIndex == other.m_fileIndex && m_registry == other.m_registry;
}
