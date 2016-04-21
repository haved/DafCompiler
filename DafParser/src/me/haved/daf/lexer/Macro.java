package me.haved.daf.lexer;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class Macro {
	private String name;
	private String[] parameters; //null means no parameters
	private String definition; //null means no definition
	
	private Macro(String name, String[] parameters, String definition) {
		this.name = name;
		this.parameters = parameters != null && parameters.length > 0 ? parameters : null;
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
	
	public int getMacroParameterCount() {
		return parameters == null ? 0:parameters.length;
	}
	
	/**
	 * @return the definition of the macro, or null if there isn't one
	 */
	public String getMacroValue() {
		return definition;
	}
	
	public boolean hasValue() {
		return definition != null;
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
	
	public MacroMap makeMacroMapFromParameters(String[] params) {
		if(params.length != parameters.length) {
			log(ERROR, "Wrong amount of parameters to macro %s! Expected %d, got %d!", name, parameters.length, params.length);
			return null;
		}
		MacroMap map = new MacroMap();
		for(int i = 0; i < params.length; i++)
			map.tryAddMacro(new Macro(parameters[i], null, params[i])); //parameters are the names, while params are the definitions
		return map;
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
				
				if(!TextParserUtil.isNormalWhitespace(c) & !TextParserUtil.isStartOfMacroParameters(c)) { //Must be space or '<' after name!
					log(ERROR, "Found unknown character after macro name! '%c'", c);
					return null;
				}
				break;
			}
		}
		
		if(macroName == null) {
			return new Macro(text, null, null); //Give all of the name as macro identifier
		}
		
		while(TextParserUtil.isNormalWhitespace(text.charAt(i)))
			i++; //Skip all whitespace
		
		ArrayList<String> parameters = new ArrayList<>();
		if(TextParserUtil.isStartOfMacroParameters(text.charAt(i))) { //Argument list! Hurrah!
			i++; //To get past the '<'
			boolean lookingForParam = true; //Means you are looking for a pound symbol or '>'
			boolean doneWithParam  = false; //Means you are looking for a special char between two parameters or '>'
			int parameterNameStart = 0; //The first letter of the current parameter name
			
			for(; i < text.length(); i++) {
				char c = text.charAt(i);
				if(TextParserUtil.isEndOfMacroParameters(c) & (lookingForParam || doneWithParam))
					break; //We are done with the parameter list
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
						log(ERROR, "Something NOT a pound symbol ( %c ) found when looking for a macro parameter", c);
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
					if(TextParserUtil.isNormalWhitespace(c)) //We go to i++;
						continue;
					doneWithParam = false;
					lookingForParam = true;
					if(TextParserUtil.isEndOfMacroParameters(c)) //This means we are done with the parameter list
						break;
					else if(TextParserUtil.isPoundSymbol(c)) { //We found another parameter before a separator!
						log(ERROR, "Found a pound symbol after the name of a macro parameter");
						return null;
					}
					else if(!TextParserUtil.isLegalSpecialCharacter(c)) { //We found some character that is special, but not legal as a separator!
						log(ERROR, "Found illegal charcter between parameter names");
						return null;
					}
				}
			}
			i++; //Get past the last char of the list
			
			if(i < text.length() && !TextParserUtil.isAnyWhitespace(text.charAt(i))) {
				log(ERROR, "Char found right after a macro parameter list: '%c'", text.charAt(i));
				return null;
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
		
		int indexOfDefStart = text.indexOf(TextParserUtil.ENCLOSE_MACRO, i); //TRUE: If charAt(i) is '{', i is returned!
		int endOfLine = text.indexOf(TextParserUtil.END_OF_LINE, i); //The endOfTheLine index itself is "outside the line"
		if(endOfLine == -1)
			endOfLine = text.length(); //We just go on for the whole line + 1
		
		if(endOfLine < indexOfDefStart) {
			log(ERROR, "The end of the macro definition line was found before a supplied opening char (%c)! Too much text was sent!", TextParserUtil.ENCLOSE_MACRO);
			return null;
		}
		if(indexOfDefStart == -1) //We are going from i to endOfLine;
			definition = text.substring(i, endOfLine);
		else if(indexOfDefStart == i) {
			
			int indexOfClose = text.indexOf(TextParserUtil.ENCLOSE_MACRO, indexOfDefStart+1);
			if(indexOfClose == -1) {
				log(ERROR, "Macro definition opened but not closed with '%c'", TextParserUtil.ENCLOSE_MACRO);
				return null;
			}
			definition = text.substring(indexOfDefStart+1, indexOfClose);
		}
		else {
			log(ERROR, "The macro definition contained a opening char (%c), but there was text before it!", TextParserUtil.ENCLOSE_MACRO);
			return null;
		}
			
		if(parameters.size() == 0)
			return new Macro(macroName, null, definition);
		
		String[] params = new String[parameters.size()];
		return new Macro(macroName, parameters.toArray(params), definition);
	}
}
