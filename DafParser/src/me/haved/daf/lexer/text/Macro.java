package me.haved.daf.lexer.text;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class Macro {
	
	public static final char MACRO_DEFINITION_PERIMETER = '$';
	
	private String name;
	private String[] parameters;
	private char[] separators;
	private String definition;
	
	private Macro(String name, String[] parameters, char[] separators, String definition) {
		logAssert(name!=null && !name.trim().isEmpty());
		this.name = name;
		this.parameters = parameters != null && parameters.length > 0 ? parameters : null;
		this.separators = separators != null && separators.length > 0 ? separators : null;
		this.definition = definition != null && !definition.isEmpty() ? definition : null;
		
		if(this.separators != null && this.parameters.length > 1)
			logAssert(this.separators != null && this.separators.length == this.parameters.length -1);
		if(this.separators != null)
			logAssert(this.parameters != null && this.separators.length == this.parameters.length -1);
		
		StringBuilder builder = new StringBuilder();
		
		builder.append(String.format("Name: %s%n", name));
		builder.append("<");
		
		for(int i = 0; parameters != null && i < parameters.length; i++) {
			builder.append(parameters[i]).append("  ");
			if(separators != null && i < separators.length)
				builder.append(separators[i]);
			builder.append("\n");
		}
		builder.append(String.format(">%nDefinition: %s%n", definition));
		
		println(builder.toString());
	}
	
	public String getName() {
		return name;
	}
	
	public static Macro makeMacroFromString(String text) {
		int index = 0;
		while(index < text.length() && TextParserUtil.isNormalWhitespace(text.charAt(index))) //Skip whitespaces
			index++;
		
		if(index >= text.length()) {
			log(ERROR, "Macro definition didn't even contain a name!");
			log(SUGGESTION, "Add a name! Example: '#macro MyMacro'");
			return null;
		}
		
		//Now at start of name
		
		if(!TextParserUtil.isStartOfIdentifier(text.charAt(index))) {
			log(ERROR, "A macro name may not start with the char '%c'", text.charAt(index));
			log(SUGGESTION, "Start you macro name with a letter or an underscore");
			return null;
		}
		
		int startOfName = index;
		
		while(index < text.length() && TextParserUtil.isIdentifierChar(text.charAt(index)))
			index++;
		
		if(index == startOfName) {
			log(ERROR, "The macro definition didn't have a name!");
		}
		
		String macroName = text.substring(startOfName, index);
		
		while(index < text.length() && TextParserUtil.isNormalWhitespace(text.charAt(index))) //Skip whitespaces
			index++;
		
		if(index >= text.length()) { //Are we done yet? In which case we only have a name
			return new Macro(macroName, null, null, null);
		}
		
		ArrayList<String> parameters = new ArrayList<>();
		StringBuilder separators = new StringBuilder();
		if(TextParserUtil.isStartOfMacroParameters(text.charAt(index))) { //We've got parameters!
			boolean lookingForParameter = true;
			int startOfParam = -1;
			
			index++;
			
			while(index < text.length()) {
				char c = text.charAt(index);
				if(startOfParam<0 && TextParserUtil.isEndOfMacroParameters(c))
					break;
				else if(lookingForParameter && !TextParserUtil.isNormalWhitespace(c)) {
					if(!TextParserUtil.isStartOfIdentifier(c)) {
						log(ERROR, "A macro parameter must start with a letter or underscore, not '%c'", c);
						return null;
					}
					startOfParam = index;
					lookingForParameter = false;
				}
				else if(startOfParam >= 0 && !TextParserUtil.isIdentifierChar(c)) {
					parameters.add(text.substring(startOfParam,index));
					startOfParam = -1;
				}
				
				if(startOfParam < 0 && !lookingForParameter) { // lookingForParameter has to be false, but good code is self-descriptive
					if(TextParserUtil.isEndOfMacroParameters(c)) {
						continue;
					}
					else if(TextParserUtil.isLegalMacroParameterSeparator(c)) {
						separators.append(c);
						lookingForParameter = true;
					} else if(!TextParserUtil.isNormalWhitespace(c)) {
						if(TextParserUtil.isStartOfMacroParameters(c))
							log(ERROR, "Found the char '%c', the start of a new macro parameter, before a separator was found!");
						else
							log(ERROR, "The special char '%c' was found in a macro parameter list, but can't be a separator");
						return null;
					}
				}
				
				index++;
			}
		}
		
		//Parameters and separators found
		
		while(index < text.length() && TextParserUtil.isNormalWhitespace(text.charAt(index))) //Skip whitespaces
			index++;
		
		if(index >= text.length() | true)
			return new Macro(macroName, parameters.toArray(new String[parameters.size()]), separators.toString().toCharArray(), null);
		
		return null;
	}
}
