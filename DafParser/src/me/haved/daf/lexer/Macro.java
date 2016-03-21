package me.haved.daf.lexer;

// #macro Ball true
// #macro Bull<> {false}
// #macro Func <#param> {#param}
// 

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class Macro {
	private String name;
	private String[] parameters; //null means no parameters
	private String definition; //null means no definition
	
	private Macro(String name, String[] parameters, String definition) {
		this.name = name;
		this.parameters = parameters != null && parameters.length > 0 ? parameters : null;
		definition = definition.trim();
		this.definition = definition != null && definition.length()>0 ? definition : null;
	}
	
	public String getMacroName() {
		return name;
	}
	
	/**
	 * @return the parameters of the macro, or null if there are none
	 */
	public String[] getMacroParameters() {
		return parameters;
	}
	
	public boolean hasMacroParameters() {
		return parameters != null;
	}
	
	/**
	 * @return the definition of the macro, or null if there isn't one
	 */
	public String getMacroValue() {
		return definition;
	}
	
	public String toString() {
		StringBuilder builder = new StringBuilder();
		if(!hasMacroParameters())
			builder.append("null");
		else {
			builder.append("{");
			for(int i = 0; i < parameters.length; i++) {
				if(i!=0)
					builder.append(", ");
				builder.append('"').append(parameters[i]).append('"');
			}
			builder.append("}");
		}
		
		return String.format("Macro(name:\"%s\", parameters:\"%s\", value:\"%s\")", getMacroName(), builder.toString(), getMacroValue());
	}
	
	
	//Static procedures bellow here
	
	public static Macro getMacroFromString(String text) {
		if(text == null || (text = text.trim()).length() < 1) {
			log(ERROR, "Trying to add a macro without any text!");
			return null;
		}
		
		if(!TextParserUtil.isStartOfIdentifier(text.charAt(0))) {
			log(ERROR, "Macro name didn't start with a legal identifier character! '%c'", text.charAt(0));
			return null;
		}
		
		String macroName = null;
		
		int i;
		for(i = 1; i < text.length(); i++) {
			char c = text.charAt(i);
			if(!TextParserUtil.isIdentifierChar(c)) {
				macroName = text.substring(0, i); //Not including i;
				
				if(!TextParserUtil.isNormalWhitespace(c) & !TextParserUtil.isLessThanChar(c)) { //Must be space or '<' after name!
					log(ERROR, "Found unknown character after macro name! '%c'", c);
					return null;
				}
				break;
			}
		}
		
		if(macroName == null) {
			log(WARNING, "Macro definition didn't contain more than a name: '%s'", text); //Should this be legal? It's legal in C
			return new Macro(text, null, null); //Give all of the name as macro identifier
		}
		
		while(TextParserUtil.isNormalWhitespace(text.charAt(i)))
			i++; //Skip all whitespace
		
		ArrayList<String> parameters = new ArrayList<>();
		if(TextParserUtil.isLessThanChar(text.charAt(i))) { //Argument list! Hurrah!
			i++; //To get past the '<'
			boolean lookingForParam = true;
			boolean doneWithParam  = false; //Means you are looking for a special char between two parameters
			int parameterNameStart = 0; //The first letter of the current parameter name
			
			for(; i < text.length(); i++) {
				char c = text.charAt(i);
				if(TextParserUtil.isGreaterThanChar(c)) {
					i++; //Get past the end of the list
					if(i >= text.length())
						break; //The macro has for some very odd reason got parameters but no value!
					if(!TextParserUtil.isAnyWhitespace(text.charAt(i))) {
						log(ERROR, "Char found right after a macro name: '%c'", text.charAt(i));
						return null;
					}
					break; //We are done with the parameter list
				}
				if(lookingForParam) {
					if(TextParserUtil.isPoundSymbol(c)) {
						i++; //Go to the next char to see that there is a proper name there
						parameterNameStart = i; //First letter of the name
						lookingForParam = false;
						if(!TextParserUtil.isStartOfIdentifier(text.charAt(i))) {
							log(ERROR, "A macro parameter can't start with the character '%c'", text.charAt(i));
							return null;
						}
					} else if(!TextParserUtil.isNormalWhitespace(c)) {
						log(ERROR, "Something NOT a pound symbol ( %cw ) found when looking for a macro parameter", c);
						return null;
					}
				}
				else if(!doneWithParam) {
					if(!TextParserUtil.isIdentifierChar(c)) {
						//We are done with the parameter name! Must check earlier names;
						String paramName = text.substring(parameterNameStart, i); //I is the letter after the identifier
						if(parameters.contains(paramName)) {
							log(ERROR, "The macro parameter name '%s' was already taken!", paramName);
							return null;
						}
						parameters.add(paramName);
						doneWithParam = true; //This means we check c again
					}
				}
				
				if(doneWithParam) { //When the name is over, we are doneWithParam until we find a separator character
					if(TextParserUtil.isGreaterThanChar(c)) //This means we are done with the parameter list
						continue;
					else if(TextParserUtil.isNormalWhitespace(c)) { //We go to i++;
						continue;
					}
					else if(TextParserUtil.isPoundSymbol(c)) { //We found another parameter before a separator!
						log(ERROR, "Found a pound symbol after the name of a macro parameter");
					}
					else if(!TextParserUtil.isLegalSpecialCharacter(c)) { //We found some character that is special, but not legal as a separator!
						log(ERROR, "Found illegal charcter between parameter names");
						return null;
					}
					doneWithParam = false;
					lookingForParam = true;
				}
			}
		}
		
		if(i >= text.length()) {
			if(parameters.size() == 0) {
				log(MESSAGE, "The macro named '%s' has got no definitions, and an empty parameter list!", macroName);
				return new Macro(macroName, null, null);
			}
			else {
				log(MESSAGE, "The macro named '%s' has got %d parameters, but no definition!", macroName, parameters.size());
				String[] params = new String[parameters.size()];
				return new Macro(macroName, parameters.toArray(params), null);
			}
		}
		
		// Now we have the macroName, the parameters, and we know there is more to come!
		// Let's check if we have a definition that ends at the end of the line, or if it goes on until a '}'
		
		String definition = null;
		
		// First we skip all normal whitespace. I will be the first letter
		
		while(TextParserUtil.isNormalWhitespace(text.charAt(i)))
			i++;
		
		int indexOfDefStart = text.indexOf(TextParserUtil.OPEN_CURLY_BRACKETS, i); //TRUE: If charAt(i) is '{', i is returned!
		
		if(indexOfDefStart == i) {
			int previousChar = i; //Where the last curly bracket was
			int level = 1; //How deep in curly brackets we are. 0 is outside the definition
			while(true) {
				int indexOfOpen;
				int indexOfClose;
				if(previousChar+1>=text.length()) { //The last bracket was the end of the text, no more brackets
					indexOfOpen  = -1;
					indexOfClose = -1;
				} else {
					indexOfOpen = text.indexOf(TextParserUtil.OPEN_CURLY_BRACKETS, previousChar+1);
					indexOfClose = text.indexOf(TextParserUtil.CLOSE_CURLY_BRACKETS, previousChar+1);
				}
				
				if(indexOfClose >= 0 && (indexOfOpen < 0 || indexOfClose < indexOfOpen)) {
					previousChar = indexOfClose;
					level--;
					
					if(level <= 0) {
						assert(level == 0);
						if(indexOfClose != text.length()-1) {
							log(ERROR, "More stuff found after last closing curly bracket!: '%s'", text.substring(indexOfClose));
							return null;
						}
						definition = text.substring(i+1, indexOfClose); //Not including either bracket
						break;
					}
				}
				else if(indexOfOpen >= 0 && (indexOfClose < 0 || indexOfOpen < indexOfClose)) {
					previousChar = indexOfOpen;
					level++;
				} else { //No more brackets in the text
					log(ERROR, "The opening curly bracket of a macro definition wasn't properly closed! Levels in: %d", level);
					return null;
				}
			}
		} else {
			int endOfLine = text.indexOf(TextParserUtil.END_OF_LINE, i); //The endOfTheLine index itself is outside the line
			if(endOfLine == -1)
				endOfLine = text.length(); //We just go on for the whole line + 1
			
			if(endOfLine < indexOfDefStart) {
				log(ERROR, "The end of the macro definition line was found before a supplied opening curly bracket! Too much text was sent!");
				return null;
			}
			if(indexOfDefStart == -1) //We are going from i to endOfLine;
				definition = text.substring(i, endOfLine);
			else {
				log(ERROR, "The macro definitin contained a opening curly bracket, but there were none at the beginning!");
				return null;
			}
		}
			
		if(parameters.size() == 0)
			return new Macro(macroName, null, definition);
		
		String[] params = new String[parameters.size()];
		return new Macro(macroName, parameters.toArray(params), definition);
	}
}
