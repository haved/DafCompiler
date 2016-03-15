package me.haved.daf.lexer;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	
	
	public static ArrayList<Token> tokenizeFile(File inputFile, String infileName, MacroMap map) {
		try {
			StringBuilder fileText = new StringBuilder();
			try(BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				String s;
				while((s=reader.readLine())!=null) {
					fileText.append(s);
					fileText.append("\n");
				}
			}
			
			String text = fileText.toString();
			
			text = evaluateMacros(text, map);
			
			ArrayList<Token> tokens = new ArrayList<>();
			
			return tokens;
		}
		catch(Exception e) {
			e.printStackTrace();
			log(infileName, FATAL_ERROR, "Some sort of Exception was thrown in tokenizeFile()");
			return null;
		}
	}
	
	public static String evaluateMacros(String string, MacroMap map) {
		StringBuilder builder = new StringBuilder();
		
		for(int i = 0; i < string.length(); i++) {
			if(string.charAt(i)=='#'){
				int start = i;
				do //Using do - while!
					i++;
				while(TextParserUtil.isIdentifierChar(string.charAt(i)));
				
				String name = string.substring(start+1, i);
				log(DEBUG, "Found pound symbol text: '%s'", name);
			}
		}
		
		return builder.toString();
	}
}
