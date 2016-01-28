package me.haved.dafParser.lexical;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;

import static me.haved.dafParser.LogHelper.*;

public class LexicalParser {
	private static HashMap<String, TokenType> types;
	
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
	
	private static final int UNKOWN_TYPE = 0;
	private static final int KEYWORD_TYPE = 1;
	private static final int INTEGER_LIT_TYPE = 2;
	private static final int REAL_LIT_TYPE = 3;
	private static final int FLOAT_LIT_TYPE = 4;
	private static final int STRING_LIT_TYPE = 5;
	private static final int COMPILER_TYPE_1 = 6;
	private void parseFromReader(BufferedReader reader) throws Exception {
		StringBuilder word = new StringBuilder();
		int wordType = UNKOWN_TYPE;
		char in;
		int line = 1;
		int col = 0;
		while(true) {
			int i = reader.read();
			if(i==-1)
				in='\n';
			else
				in = (char) i;
			col++;
			
			if(wordType==UNKOWN_TYPE); //Just to skip to the bottom if it's 0
			else if(wordType==KEYWORD_TYPE) {
				if(isIdentifierChar(in))
					word.append(in);
				else {
					String keyword = word.toString();
					wordType = UNKOWN_TYPE;
					word.setLength(0); //Clear the word
					if(tryAddingTokenToList(keyword, infileName, line, col))
						tokens.add(new Token(TokenType.IDENTIFIER, new TokenFileLocation(infileName, line, col), keyword));
				}
			}
			else if(wordType==COMPILER_TYPE_1) {
				if(!isCharCompilerPound(in)) {
					log(fileLocation(infileName, line, col), ERROR, "Single compiler flag sign!");
				} else {
					word.append(in);
					while(true) {
						i = reader.read();
						if(i == -1)
							in='\n';
						else
							in = (char) i;
						col++;
						if(!isWhitespace(in))
							word.append(in);
						else {
							log(fileLocation(infileName, line, col), MESSAGE, "Compiler message: %s found.", word.toString());
							word.setLength(0);
							wordType = 0;
							break;
						}
						if(i==-1)
							break;
					}
					if(i == -1)
						break;
				}
				word.setLength(0);
				wordType=UNKOWN_TYPE;
			}
			
			if(i == -1)
				break;
			
			if(wordType==UNKOWN_TYPE) {
				if(isCharCompilerPound(in)) {
					wordType = COMPILER_TYPE_1;
					word.append(in);
				}
				else if(isLetterOrUnderscore(in)) {
					wordType = KEYWORD_TYPE;
					word.append(in);
				}
				else {
					if(!isWhitespace(in))
						log(fileLocation(infileName, line, col), ERROR, "Unrecogniced char: %c", in);
				}
			}
			if(isNewline(in)) {
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
	
	private static boolean isLetterOrUnderscore(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_';
	}
	
	private static boolean isIdentifierChar(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_' || (c >= '0' && c <= '9');
	}
	
	private static boolean isCharCompilerPound(char c) {
		return c == '#';
	}
	
	private static boolean isWhitespace(char c) {
		return c == ' ' || c == '\t' || c=='\n' || c=='\r';
	}
	
	private static boolean isNewline(char c) {
		return c == '\n';
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
