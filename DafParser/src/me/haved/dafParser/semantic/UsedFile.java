package me.haved.dafParser.semantic;

import java.io.File;
import java.util.HashMap;

import me.haved.dafParser.lexical.LexicalParser;

public class UsedFile {
	
	private File inputFile;
	private String infileName;
	
	private UsedFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
	}
	
	public void parse() {
		LexicalParser lexer = LexicalParser.getInstance(inputFile, infileName);
		lexer.parse();
	}
	
	private static HashMap<String, UsedFile> usedFiles = new HashMap<>();
	
	public static UsedFile getInstance(File inputFile, String infileName) throws Exception {
		String fileId = inputFile.getCanonicalPath();
		if(usedFiles.containsKey(fileId))
			return usedFiles.get(fileId);
		UsedFile instance = new UsedFile(inputFile, infileName);
		usedFiles.put(fileId, instance);
		return instance;
	}
}
