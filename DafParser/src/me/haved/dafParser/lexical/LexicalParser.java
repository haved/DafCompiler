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
	private static final int COMPILER_TYPE = 6;
	private void parseFromReader(BufferedReader reader) throws Exception {
		StringBuilder word = new StringBuilder();
		int wordType = UNKOWN_TYPE;
		char in;
		while(true) {
			int i = reader.read();
			if(i==-1)
				break;
			in = (char) i;
			
			if(wordType==UNKOWN_TYPE) {
				if(isLetterOrUnderscore(in)) {
					wordType = KEYWORD_TYPE;
					word.append(in);
				}
				continue;
			}
			
			if(wordType==KEYWORD_TYPE) {
				if(isIdentifierChar(in))
					word.append(in);
				else {
					String keyword = word.toString();
					wordType = UNKOWN_TYPE;
					word.setLength(0); //Clear the word
					log(String.format("%s:%d:%d",infileName,0,0), MESSAGE, "Picked out keyword: %s", keyword);
				}
			}
		}
		log(infileName, MESSAGE, "Parsed text: %n%s", word.toString());
	}
	
	private static boolean isLetterOrUnderscore(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_';
	}
	
	private static boolean isIdentifierChar(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_' || (c >= '0' && c <= '9');
	}
	
	private static void fillTypes() {
		if(types!=null)
			return;
		types = new HashMap<>();
		for(TokenType type:TokenType.values()) {
			String keyword = type.getKeyword();
			logAssert(types.containsKey(keyword) == false, 
					String.format("The keyword %s is registered twice in Lexical Parser.", keyword));	
			types.put(keyword, type);
		}
	}
}
