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
	private ArrayList<Token> tokens;
	
	public LexicalParser(File file) {
		this.inputFile = file;
		tokens = new ArrayList<>();
	}
	
	public void parse() {
		if(LexicalParser.types==null)
			LexicalParser.fillTypes();
		logAssert(inputFile.isFile(), "File given to LexicalParser doesn't exist");
		
		try {
			try(BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				parseFromReader(reader);
			}
		}
		catch(Exception e) {
			e.printStackTrace(out);
			log(FATAL_ERROR, "LexicalParser.parse(): exception thrown");
		}
	}
	
	private void parseFromReader(BufferedReader reader) {
		while(true) {
			
		}
	}
	
	private static void fillTypes() {
		for(TokenType type:TokenType.values()) {
			String keyword = type.getKeyword();
			logAssert(types.containsKey(keyword) == false, 
					String.format("The keyword %s is registered twice in Lexical Parser.", keyword));	
			types.put(keyword, type);
		}
	}
}
