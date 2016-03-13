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
	
	/** Add a macro to the list based on the token
	 * 
	 * @param token the token used for the text and the error information
	 */
	public void addMacro(Token token) {
		addMacro(token, token.getTokenContents());
	}
	
	/** Add a macro to the list from a string of text
	 * 
	 * @param token is used for error messages
	 * @param text is used to find the name and meaning of the macro
	 */
	private void addMacro(Token token, String text) {
		String name = getNameFromText(token, text);
		if(name==null)
			return;
		
		if(macros.containsKey(name))
			log(token, WARNING, "The macro %s was already defined! Overriding!", name);
		
		int startOfMeaning;
		if(text.indexOf("<") < text.indexOf(" ")) //We are using parameters!
			startOfMeaning = text.indexOf(">")+1; //We know from getNameFromText that there is one!
		else
			startOfMeaning = text.indexOf(" ")+1;
		macros.put(name, text.substring(startOfMeaning).trim());
	}
	
	/** Get the meaning of a macro, if the specified text is a macro
	 * 
	 * @param token The token used for error messages
	 * @param text The text you want to check
	 * @return the meaning of the macro, or null if it wasn't a macro
	 */
	public String tryUseMacro(Token token, String text) {
		String name = getNameFromText(token, text);
		
		if(macros.containsKey(name)) {
			log(token, DEBUG, "A macro was found! '%s'", text);
		}
		
		return null;
	}
	
	private String getNameFromText(Token token, String text) {
		int indexOfLessThan = text.indexOf("<");
		int indexOfSpace = text.indexOf(" ");
		
		if (indexOfLessThan == -1 || indexOfSpace < indexOfLessThan) {
			return text.substring(0, indexOfSpace); //No parameters, name is just the first word
		}
		else {
			int indexOfGreaterThan = text.indexOf(">", indexOfLessThan);
			if(indexOfGreaterThan == -1) {
				log(token, ERROR, "Macro parameter list not closed. Expecting '>'!");
				return null; 
			}
			
			String params = text.substring(indexOfLessThan+1, indexOfGreaterThan).trim();
			String[] paramNames = params.split(",");
			for(int i = 0; i < paramNames.length; i++) {
				if(containsWhitespace(paramNames[i].trim())) {
					log(token, ERROR, "Macro parameter '%s' contained a whitespace!", paramNames[i]);
					return null;
				}
			}
			return String.format("%s<%d", text.substring(0, indexOfLessThan), paramNames.length); //Name + lessThan + numParams: Example: MakeString<2 
		}
	}
}
