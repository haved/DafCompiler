package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import me.haved.dafParser.lexical.LexicalParser;
import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.semantic.Definition;
import me.haved.dafParser.semantic.KnownOfClass;

import static me.haved.dafParser.LogHelper.*;

public class InputFile {

	protected static final int UNPARSED = 0;
	protected static final int PARSING = 1;
	protected static final int PARSED = 2;
	
	protected File inputFile;
	protected String infileName;
	
	protected ArrayList<Definition> definitions;
	protected ArrayList<KnownOfClass> knownOfClasses;
	
	protected int parsingProgress;
	
	protected InputFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		
		this.definitions = new ArrayList<>();
		this.knownOfClasses = new ArrayList<>();
		
		this.parsingProgress = UNPARSED;
	}
	
	public void parse() {
		if(parsingProgress==PARSING) //Already parsing!
			throw new AlreadyParsingException(infileName);
		if(parsingProgress==PARSED) //Already parsed
			return;
		parsingProgress=PARSING;
		
		LexicalParser lexer = LexicalParser.getInstance(inputFile, infileName);
		lexer.parse();
		ArrayList<Token> tokens = lexer.getTokens();
		
		goThroughTokens(tokens);
		
		terminateIfErrorsLogged();
		parsingProgress = PARSED;
	}
	
	protected void goThroughTokens(ArrayList<Token> tokens) {
		
	}
	
	private static HashMap<String, InputFile> inputFiles = new HashMap<>();
	
	public static InputFile getInstance(File inputFile, String infileName) throws Exception {
		String fileId = inputFile.getCanonicalPath();
		if(inputFiles.containsKey(fileId))
			return inputFiles.get(fileId);
		InputFile instance = new InputFile(inputFile, infileName);
		inputFiles.put(fileId, instance);
		return instance;
	}
	
	public static InputFile getMainFileInstance(File inputFile, String infileName) throws Exception {
		String fileId = inputFile.getCanonicalPath();
		if(inputFiles.containsKey(fileId)) {
			InputFile instance = inputFiles.get(fileId);
			if(instance instanceof MainInputFile)
				return (MainInputFile) instance;
		}
		MainInputFile instance = new MainInputFile(inputFile, infileName);
		inputFiles.put(fileId, instance); //Overriding any previous implementation, but UF and LP remember
		return instance;
	}
}