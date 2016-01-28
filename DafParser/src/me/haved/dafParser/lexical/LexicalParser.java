package me.haved.dafParser.lexical;

import java.awt.DefaultFocusTraversalPolicy;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;

import static me.haved.dafParser.LogHelper.*;

public class LexicalParser {
	private static HashMap<String, TokenType> types;
	private static TokenParser[] defaultParsers = new TokenParser[] {CompilerTokenParser.instance};
	
	private File inputFile;
	private String infileName;
	private ArrayList<Token> tokens;
	
	public LexicalParser(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		tokens = new ArrayList<>();
	}
	
	public void parse() {
		LexicalParser.fillTypes();
		logAssert(inputFile.isFile(), "File given to LexicalParser doesn't exist");
		
		try {
			try(BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				parseFromReader(reader);
			}
			catch(Exception e) {
				throw e;
			}
		}
		catch(Exception e) {
			e.printStackTrace(out);
			log(FATAL_ERROR, "LexicalParser.parse(): exception thrown");
		}
		log(INFO, "Finished Lexical Parsing...");
	}
	
	private void parseFromReader(BufferedReader reader) throws Exception {
		TokenParser parser=null;
		int input;
		char character;
		int line = 1;
		int col = 0;
		
		while(true) {
			col++;
			input = reader.read();
			character = input==-1?'\n':(char) input;
			
			if(parser!=null) {
				boolean status = parser.parse(character, line, col);
				if(!status) {
					tokens.add(parser.getReturnedToken());
					parser = null;
				}
			}
			if(parser==null) {
				for(int i = 0; i < defaultParsers.length; i++) {
					if(defaultParsers[i].tryStartParsing(character, infileName, line, col)) {
						parser = defaultParsers[i];
					}
				}
			}
			
			if(input==-1)
				break;
			if(TokenParser.isNewline(character)) {
				col = 0;
				line++;
			}
		}
		terminateIfErrorsLogged();
		log(fileLocation(infileName, line, col), MESSAGE, "Finished Lexical Parsing with %d tokens!", tokens.size());
	}
	
	private boolean tryAddingTokenToList(String keyword, String infileName, int line, int col) {
		if(LexicalParser.types.containsKey(keyword))
			tokens.add(new Token(LexicalParser.types.get(keyword), new TokenFileLocation(infileName, line, col), null));
		else
			return false;
		return true;
	}
	
	private static void fillTypes() {
		if(types!=null)
			return;
		types = new HashMap<>();
		for(TokenType type:TokenType.values()) {
			if(type.isSpecial())
				continue;
			String keyword = type.getKeyword();
			logAssert(types.containsKey(keyword) == false, 
					String.format("The keyword %s is registered twice in Lexical Parser.", keyword));	
			types.put(keyword, type);
		}
	}
}
