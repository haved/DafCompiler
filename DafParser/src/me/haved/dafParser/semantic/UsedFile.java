package me.haved.dafParser.semantic;

import java.io.File;
import java.util.HashMap;

import me.haved.dafParser.lexical.LexicalParser;

public class UsedFile {
	
	private static final int UNPARSED = 0;
	private static final int PARSING = 1;
	private static final int PARSED = 2;
	
	private File inputFile;
	private String infileName;
	private int parsingProgress;
	
	private UsedFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		this.parsingProgress = UNPARSED;
	}
	
	public void parse(boolean publicOnly) {
		if(parsingProgress==PARSING) //Already parsing!
			throw new AlreadyParsingException(infileName);
		if(parsingProgress==PARSED) //Already parsed or failed
			return;
		parsingProgress=PARSING;
		
		LexicalParser lexer = LexicalParser.getInstance(inputFile, infileName);
		lexer.parse();
		
		TokenDigger digger = new TokenDigger(lexer.getTokens());
	}
	
	public boolean isParsed() {
		return parsingProgress == PARSED;
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
