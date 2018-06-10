#pragma once

class FileRegistry;

struct RegisteredFileInternal;

class RegisteredFile {
private:
	FileRegistry* m_registry;
	int m_fileIndex;
public:
	inline RegisteredFile(FileRegistry* registry, int fileIndex) : m_registry(registry), m_fileIndex(fileIndex) {}
	const RegisteredFileInternal& get() const; //Defined in FileController.cpp
	bool operator ==(const RegisteredFile& other) const;
};
