package me.haved.daf.lexer;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	
	public static ArrayList<Token> tokenizeFile(File inputFile, String infileName, MacroMap map) {
		try {
			StringBuilder fileText = new StringBuilder('\n'); //Start of with a newline
			try(BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				String s;
				while((s=reader.readLine())!=null) {
					fileText.append(s);
					fileText.append("\n"); //End with a newline for every line
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
			if(TextParserUtil.isPoundSymbol(string.charAt(i))){
				int start = i;
				do //Using do - while!
					i++;
				while(!TextParserUtil.isWhitespace(string.charAt(i)));
				
				String name = string.substring(start+1, i);
				log(DEBUG, "Found pound symbol text: '%s'", name);
				
				if(name.equals("macro")) {
					int nameStart = -1;
					boolean parameterList = false;
					while(true) {
						i++;
						char c = string.charAt(i);
						if(nameStart==-1) {
							if(TextParserUtil.isStartOfIdentifier(c))
								nameStart = i;
						}
						else if(parameterList && TextParserUtil.isGreaterThanChar(c)) {
							i++;
							break;
						}
						else {
							if(TextParserUtil.isWhitespace(c)) {
								if(TextParserUtil.isLessThanChar(string.charAt(i+1)))
									parameterList = true;
								else
									break;
							}
							else if(TextParserUtil.isLessThanChar(c)) {
								parameterList = true;
							}
							else if(!TextParserUtil.isIdentifierChar(c)) {
								log(ERROR, "Found illegal char in macro name!");
							}
						}
					} //After the loop, i is the char after the name
				}
			}
		}
		
		return builder.toString();
	}
}
