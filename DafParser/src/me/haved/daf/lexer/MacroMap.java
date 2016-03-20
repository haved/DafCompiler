package me.haved.daf.lexer;

import java.util.HashMap;
import java.util.HashSet;

import static me.haved.daf.LogHelper.*;

public class MacroMap {
	private HashMap<String, String> macros;
	
	public MacroMap() {
		macros = new HashMap<>();
	}
	
	public boolean tryAddMacro(String name, String definition) {
		if(name==null || name.length()==0) {
			log(ERROR, "Trying to add a macro without a name");
			return false;
		}
		
		if(!isNameValidMacroIdentifer(name)) {
			log(ERROR, "The macro name '%s' is not valid!", name);
			return false;
		}
		
		macros.put(name, definition);
		
		return true;
	}
	
	public static boolean isNameValidMacroIdentifer(String name) {
		name=name.trim();
		
		if(!TextParserUtil.isStartOfIdentifier(name.charAt(0))) {
			log(ERROR, "Macro identifier didn't start with a valid identifier keyword");
			return false;
		}
		int i;
		for(i = 1; i < name.length(); i++)
			if(!TextParserUtil.isIdentifierChar(name.charAt(i))) {
				while(TextParserUtil.isNormalWhitespace(name.charAt(i))) {
					i++;
					if(i>=name.length()) //For some reason we just passed a bunch of spaces and found nothing more
						break;
				}
				
				if(TextParserUtil.isLessThanChar(name.charAt(i)))
					break;
				log(ERROR, "Macro name contained illegal special character! '%s'", name);
				return false;
			}
		
		if(i < name.length()-1) {
			//At this point, charAt(i) == '<'
			i++;
			boolean lookingForParam = true; //This means we are looking _for_ a parameter or the end of the list
			boolean doneWithParam = false; //Before the special character between names
			
			int parameterNameStart = 0;
			
			HashSet<String> params = new HashSet<>();
			
			for(; i < name.length(); i++) {
				char c = name.charAt(i);
				if(TextParserUtil.isGreaterThanChar(c)) {
					if(i+1 < name.length()) {
						log(ERROR, "More stuff found after end of parameter list in macro name!");
						return false;
					}
					return true; //Finally done!
				}
				if(lookingForParam) {
					if(TextParserUtil.isPoundSymbol(c)) {
						lookingForParam = false;
						i++;
						if(i>=name.length()) {
							log(ERROR, "No identifier following pound symbol in macro parameter list");
							return false;
						}
						if(!TextParserUtil.isStartOfIdentifier(name.charAt(i))) {
							log(ERROR, "A macro parameter must start with a-z or A-Z or underscore");
							return false;
						}
						parameterNameStart = i; //The first letter of the macro name
					}
					else if(!TextParserUtil.isNormalWhitespace(c)) {
						log(ERROR, "Found something not whitespace before a macro parameter! '%c'", c);
						return false;
					}
				} else if(!doneWithParam) { //We are looking AT a parameter with the first letter checked
					if(!TextParserUtil.isIdentifierChar(c)) {
						//We are done with the parameter name! Must check earlier names;
						String paramName = name.substring(parameterNameStart, i); //I is the letter after the identifier
						if(params.contains(paramName)) {
							log(ERROR, "The macro parameter name '%s' was already taken!", name);
							return false;
						}
						params.add(paramName);
						doneWithParam = true; //This means we check c again
					}
				}
				if (doneWithParam) { //Right here!
					if(TextParserUtil.isGreaterThanChar(c)) 
						continue;
					else if(TextParserUtil.isNormalWhitespace(c)) {
						continue;
					}
					else if(!TextParserUtil.isLegalSpecialCharacter(c)) {
						log(ERROR, "Found illegal charcter between parameter names");
						return false;
					}
					doneWithParam = false;
					lookingForParam = true;
				}
			}
		}
		
		return true;
	}
}
