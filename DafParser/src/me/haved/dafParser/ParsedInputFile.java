package me.haved.dafParser;

import java.io.File;

import static me.haved.dafParser.LogHelper.*;

public class ParsedInputFile {
	public ParsedInputFile(File inputFile) {
		
	}
	
	public void writeToCppAndHeader(File cppFile, File headerFile) {
		log(INFO, "ParsedInputFile writing to files '%s' and '%s'", cppFile.getName(), headerFile.getName());
	}
}
