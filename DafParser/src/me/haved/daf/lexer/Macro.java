package me.haved.daf.lexer;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class Macro {
	private String name;
	private String[] parameters; //null means no parameters
	private char[]   separators; //null means no separators
	private String   definition; //null means no definition
	
	private Macro(String nameIn, String[] parametersIn, char[] separatorsIn, String definitionIn) {
		this.name = nameIn;
		this.parameters = parametersIn != null && parametersIn.length > 0 ? parametersIn : null;
		this.separators = separatorsIn != null && separatorsIn.length > 0 ? separatorsIn : null;
		this.definition = definitionIn != null && definitionIn.length()>0 ? definitionIn : null;
		
		if(separators == null && parameters != null && parameters.length!=1)
			log(ASSERTION_FAILED, "No separator chars but more than one parameter given to Macro()");
		if(separators != null && (parameters == null || separators.length+1 != parameters.length))
			log(ASSERTION_FAILED, "Macro() was given parameter and separator lists of conflicting sizes (seps:%d)", separators.length);
		
		//System.err.printf("%nMacro created: %s%n%n", toString());
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
	
	public char getSeparator(int index) {
		if(separators == null)
			log(ASSERTION_FAILED, "Tried getting parameters separator #%d from macro %s that doesn't have separators!",
					index, getMacroName());
		else if(index >= separators.length)
			log(ASSERTION_FAILED, "Tried getting parameters separator #%d from macro %s that only has %d separators!",
					index, getMacroName(), separators.length);
	
		return separators[index];
	}
	
	public String toString() {
		StringBuilder builder = new StringBuilder();
		if(!hasMacroParameters())
			builder.append("null");
		else {
			builder.append("{");
			for(int i = 0; i < parameters.length; i++) {
				if(i!=0)
					builder.append(separators[i-1]);
				builder.append('"').append(parameters[i]).append('"');
			}
			builder.append("}");
		}
		
		return String.format("Macro(name:\"%s\", parameters:\"%s\", value:\"%s\")", getMacroName(), builder.toString(), getMacroValue());
	}
	
	public MacroMap makeMacroMapFromParameters(String[] params) {
		if(params.length != (parameters == null ? 0 : parameters.length)) {
			log(ERROR, "Wrong amount of parameters to macro %s! Expected %d, got %d!", name, parameters.length, params.length);
			return null;
		}
		MacroMap map = new MacroMap();
		for(int i = 0; i < params.length; i++)
			map.tryAddMacro(String.format("%s %c%s%c", 
					parameters[i], TextParserUtil.ENCLOSE_MACRO, params[i], TextParserUtil.ENCLOSE_MACRO)); 
									//parameters are the names, while params are the definitions
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
					log(ERROR, "Found unknown character after macro name(%s): '%c'", macroName, c);
					return null;
				}
				break;
			}
		}
		
		if(macroName == null) {
			return new Macro(text, null, null, null); //Give all of the name as macro identifier
		}
		
		while(TextParserUtil.isNormalWhitespace(text.charAt(i)))
			i++; //Skip all whitespace
		
		ArrayList<String> parameters = new ArrayList<>();
		ArrayList<Character> separators = new ArrayList<>();
		if(TextParserUtil.isStartOfMacroParameters(text.charAt(i))) { //Argument list! Hurrah!
			i++; //To get past the '<'
			boolean lookingForParam  = true; //Means you are looking for an identifier to mark the beginning of a parameter
			int parameterNameStart = 0; //The first letter of the current parameter name
			
			int scope = 0;
			
			for(; i < text.length(); i++) {
				char c = text.charAt(i);
				if(TextParserUtil.isStartOfMacroParameters(c)) {
					scope++;
					continue;
				}
				if(scope > 0) {
					if(TextParserUtil.isEndOfMacroParameters(c))
						scope--;
					continue;
				}
				if(TextParserUtil.isEndOfMacroParameters(c) && lookingForParam)
					break; //We are done with the parameter list
				
				if(lookingForParam) {
					if(TextParserUtil.isStartOfIdentifier(c)) {
						parameterNameStart = i;
						lookingForParam = false;
					} else if(!TextParserUtil.isNormalWhitespace(c)) {
						log(ERROR, "A macro parameter name must start with a letter, not %c", c);
						return null;
					}
				}
				else {
					if(TextParserUtil.isLegalMacroParameterSeparator(c) | TextParserUtil.isEndOfMacroParameters(c)) {
						String parameter = text.substring(parameterNameStart, i).trim();
						
						if(!testMacroName(parameter)) {
							log(ERROR, "Not a valid macro parameter: '%s'", parameter);
							return null;
						}
						
						parameters.add(parameter);
						lookingForParam = true;
						
						if(TextParserUtil.isEndOfMacroParameters(c))
							i--;
						else
							separators.add(c);
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
				log(DEBUG, "The macro named '%s' has got no definitions, and an empty parameter list!", macroName);
				return new Macro(macroName, null, null, null);
			}
			else {
				//log(MESSAGE, "The macro named '%s' has got %d parameters, but no definition!", macroName, parameters.size());
				String[]    params = new String   [parameters.size()];
				Character[] seps   = new Character[separators.size()];
				return new Macro(macroName, parameters.toArray(params), toPrimitiveArray(separators.toArray(seps)), null);
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
			return new Macro(macroName, null, null, definition);
		
		String[] params = new String[parameters.size()];
		Character[]   seps   = new Character  [separators.size()];
		return new Macro(macroName, parameters.toArray(params), toPrimitiveArray(separators.toArray(seps)), definition);
	}
	
	private static boolean testMacroName(String text) {
		Macro macro = getMacroFromString(text);
		if(macro==null)
			return false;
		return !macro.hasValue(); //It shall not have a value!
	}
	
	private static char[] toPrimitiveArray(Character[] chars) {
		char[] output = new char[chars.length];
		
		for(int i = 0; i < output.length; i++)
			output[i] = chars[i];
		
		return output;
	}
}
