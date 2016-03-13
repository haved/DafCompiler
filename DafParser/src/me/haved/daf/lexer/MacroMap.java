package me.haved.daf.lexer;

import java.util.HashMap;

import static me.haved.daf.LogHelper.*;
import static me.haved.daf.lexer.TextParserUtil.containsWhitespace;

//    #macro Ball<Noe, Noe2, Mer> 

public class MacroMap {
	private HashMap<String, String> macros;
	
	public MacroMap() {
		macros = new HashMap<>();
	}
	
	public void addMacro(Token token) {
		addMacro(token, token.getTokenContents());
	}
	
	private void addMacro(Token token, String text) {
		String name;
		int indexOfLessThan = text.indexOf("<");
		int indexOfSpace = text.indexOf(" ");
		
		int startOfCode;
		
		if (indexOfLessThan == -1 || indexOfSpace < indexOfLessThan) {
			name = text.substring(0, indexOfSpace); //No parameters, name is just the first word
			startOfCode = indexOfSpace+1;
		}
		else {
			int indexOfGreaterThan = text.indexOf(">", indexOfLessThan);
			if(indexOfGreaterThan == -1) {
				log(token, ERROR, "Macro parameter list not closed. Expecting '>'!");
				return; 
			}
			
			startOfCode = indexOfGreaterThan+1;
			
			String params = text.substring(indexOfLessThan+1, indexOfGreaterThan).trim();
			String[] paramNames = params.split(",");
			for(int i = 0; i < paramNames.length; i++) {
				if(containsWhitespace(paramNames[i].trim())) {
					log(token, ERROR, "Macro parameter '%s' contained a whitespace!", paramNames[i]);
					return;
				}
			}
			name = String.format("%s<%d", text.substring(0, indexOfLessThan), paramNames.length); //Name + lessThan + numParams: Example: MakeString<2 
		}
		
		if(macros.containsKey(name))
			log(token, WARNING, "The macro %s was already defined! Overriding!", name);
		
		macros.put(name, text.substring(startOfCode).trim());
	}
}
