package me.haved.dafParser;

import java.io.File;
import java.util.HashMap;

public class InputFile {

	private File inputFile;
	private String infileName;
	
	private InputFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
	}
	
	public void parse() {
		
	}
	
	
	private static HashMap<String, InputFile> inputFiles = new HashMap<>();
	
	public static InputFile GetInstance(File file, String infileName) throws Exception {
		String fileId = file.getCanonicalPath();
		if(inputFiles.containsKey(fileId))
			return inputFiles.get(fileId);
		return new InputFile(file, infileName);
	}
	
	//TODO: Main Input File
}