package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import me.haved.dafParser.lexical.Token;

public class InputFile {

	private File inputFile;
	private String infileName;
	
	private InputFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
	}
	
	public void parse() {
		
	}
	
	protected void goThroughTokens(ArrayList<Token> tokens) {
		
	}
	
	private static HashMap<String, InputFile> inputFiles = new HashMap<>();
	
	public static InputFile getInstance(File file, String infileName) throws Exception {
		String fileId = file.getCanonicalPath();
		if(inputFiles.containsKey(fileId))
			return inputFiles.get(fileId);
		InputFile instance = new InputFile(file, infileName);
		inputFiles.put(fileId, instance);
		return instance;
	}
	
	//TODO: Main Input File
}