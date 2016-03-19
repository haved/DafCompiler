package me.haved.daf.lexer;

import java.util.ArrayList;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	
	public static ArrayList<Token> tokenizeFile(RegisteredFile file, MacroMap map) {
		try {
			
			log(DEBUG, file.toString());
			
			ArrayList<Token> tokens = new ArrayList<>();
			
			return tokens;
		}
		catch(Exception e) {
			e.printStackTrace();
			log(file, FATAL_ERROR, "Some sort of Exception was thrown in tokenizeFile()");
			return null;
		}
	}
	
	
}
