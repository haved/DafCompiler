package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import me.haved.dafParser.lexical.LexicalParser;
import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenFileLocation;
import me.haved.dafParser.lexical.TokenLocation;
import me.haved.dafParser.semantic.AlreadyParsingException;
import me.haved.dafParser.semantic.Definition;
import me.haved.dafParser.semantic.KnownOfClass;
import me.haved.dafParser.semantic.UsedFile;

import static me.haved.dafParser.LogHelper.*;

public class InputFile {

	protected static final int UNPARSED = 0;
	protected static final int PARSING = 1;
	protected static final int PARSED = 2;
	protected static final int FAILED = 3;
	
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
		if(parsingProgress==PARSED | parsingProgress==FAILED) //Already parsed or failed
			return;
		parsingProgress=PARSING;
		
		LexicalParser lexer = LexicalParser.getInstance(inputFile, infileName);
		lexer.parse();
		if(!lexer.isParsed()) {
			parsingProgress = FAILED;
			return; //Not parsed, mate; Give up!
		}
		ArrayList<Token> tokens = lexer.getTokens();
		
		onUsingFound(new TokenFileLocation(infileName, 0, 0), this);
		goThroughTokens(tokens);
		
		parsingProgress = PARSED;
	}
	
	protected void goThroughTokens(ArrayList<Token> tokens) {
		log(infileName, FATAL_ERROR, "InputFile.goThroughTokens is yet to be implemented");
	}
	
	protected InputFile getFileFromInclude(TokenLocation location, String includeFile) {
		return getFileFromInclude(location, includeFile, inputFile);
	}
	
	protected void onUsingFound(TokenLocation location, InputFile file) {
		try {
			UsedFile usedFile = file.getUsedFile();
			usedFile.parse(file!=this); //If you are not using yourself, only parse public stuff
			if(!usedFile.isParsed()) {
				log(location.getErrorString(), ERROR, "The used file was not found to be parsable as a UsedFile!");
				return;
			}
			addKnownOfClasses(location, usedFile.getKnownOfClasses());
		} catch(Exception e) {
			if(e instanceof AlreadyParsingException) {
				log(location.getErrorString(), ERROR, "Recursive importing of a used file!");
			}
			log(location.getErrorString(), ERROR, "Failed to get a used-file");
		}
	}
	
	protected void addKnownOfClasses(TokenLocation include, ArrayList<KnownOfClass> newKnownOfs) {
		start:
		for(KnownOfClass newKnownOf:newKnownOfs) {
			for(KnownOfClass alreadyKnownOf:knownOfClasses) {
				if(newKnownOf.name.equals(alreadyKnownOf.name)) {
					log(include.getErrorString(), ERROR, "Class name '%s' already defined!", newKnownOf.name);
					continue start;
				}
			}
			knownOfClasses.add(newKnownOf);
		}
	}
	
	public boolean isParsed() {
		return parsingProgress == PARSED;
	}
	
	public ArrayList<Definition> getDefinitions() {
		return definitions;
	}
	
	public UsedFile getUsedFile() throws Exception {
		return UsedFile.getInstance(inputFile, infileName);
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
	
	public static MainInputFile getMainFileInstance(File inputFile, String infileName) throws Exception {
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
	
	public static InputFile getFileFromInclude(TokenLocation location, String includeFile, File thisFile) {
		int thisFileLength = thisFile.getName().length();
		int thisPathLength = thisFile.getPath().length();
		String thisPath = thisFile.getPath().substring(0, thisPathLength-thisFileLength);
		File returnFile = new File(thisPath+includeFile);
		log(location.getErrorString(), DEBUG, "The '#include \"%s\"' in %s turned into the path %s", includeFile, thisFile.getParentFile(), returnFile.getPath());
		try {	
			return InputFile.getInstance(returnFile, includeFile);
		} catch(Exception e) {
			log(location.getErrorString(), ERROR, "Failed to get an InputFile instance for the file %s", returnFile.getPath());
			return null;
		}
	}
}