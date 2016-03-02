package me.haved.dafParser.semantic;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import me.haved.dafParser.InputFile;
import me.haved.dafParser.lexical.LexicalParser;
import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenType;

import static me.haved.dafParser.LogHelper.*;

public class UsedFile {
	
	private static final int UNPARSED = 0;
	private static final int PARSING = 1;
	private static final int PARSED = 2;
	
	private File inputFile;
	private String infileName;
	private int parsingProgress;
	private boolean parsedPublicOnly;
	
	private ArrayList<KnownOfClass> knownOfClasses;
	
	private UsedFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		this.parsingProgress = UNPARSED;
		knownOfClasses = new ArrayList<>();
	}
	
	public void parse(boolean publicOnly) {
		if(parsingProgress==PARSING) //Already parsing!
			throw new AlreadyParsingException(infileName);
		if(parsingProgress==PARSED && (parsedPublicOnly && !publicOnly)) //Already parsed or failed
			return;
		parsingProgress=PARSING;
		parsedPublicOnly = publicOnly;
		knownOfClasses.clear();
		
		LexicalParser lexer = LexicalParser.getInstance(inputFile, infileName);
		lexer.parse();
		if(!lexer.isParsed())
			return; //This will mean isParsed is false
		
		TokenDigger digger = new TokenDigger(lexer.getTokens());
		
		while(digger.hasMore()) {
			Token current = digger.currentAndAdvance();
			if(current.getType() == TokenType.DAF_IMPORT) {
				String fileName = current.getText();
				try {
					UsedFile file = InputFile.getFileFromInclude(current.getLocation(), fileName, inputFile).getUsedFile();
					file.parse(true);
					if(!file.isParsed()) {
						log(current.getErrorLoc(), ERROR, "Failed to compile a list of classes for the import '%s'", fileName);
						continue; //Will still continue even if this one import fails
					}
					ArrayList<KnownOfClass> importedKnownOfs = file.getKnownOfClasses();
					for(KnownOfClass knownOf:importedKnownOfs) {
						addKnownOfClass(knownOf);
					}
				} catch (Exception e) {
					log(current.getErrorLoc(), ERROR, "Failed to get a UsedFile for the import '%s'", fileName);
					return;
				}
			}
		}
		
		parsingProgress = PARSED;
	}
	
	private boolean addKnownOfClass(KnownOfClass clazz) {
		for(KnownOfClass alreadyKnownOf:knownOfClasses) {
			if(clazz.name.equals(alreadyKnownOf.name)) //Name taken already
				return false;
		}
		knownOfClasses.add(clazz);
		return true;
	}
	
	public boolean isParsed() {
		return parsingProgress == PARSED;
	}
	
	public ArrayList<KnownOfClass> getKnownOfClasses() {
		return knownOfClasses;
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
