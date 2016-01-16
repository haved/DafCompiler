package me.haved.dafParser;

import java.io.File;

import static me.haved.dafParser.LogHelper.*;

public class ParsedInputFile {
	private File inputFile;
	private String infileName;
	private boolean parsed;
	public ParsedInputFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
	}
	
	public void parse() {
		logAssert(inputFile.isFile(), "ParsedInputFile git a file that doesn't exist! Should never happen!");
		parsed = true;
	}
	
	public void writeToCppAndHeader(File cppFile, File headerFile) {
		logAssert(parsed, "Trying to write a non parsed daf file to .cpp and .h");
		log(infileName, INFO, "ParsedInputFile writing to files '%s' and '%s'", cppFile.getName(), headerFile.getName());
	}
}