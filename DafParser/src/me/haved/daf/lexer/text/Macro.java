package me.haved.daf.lexer.text;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class Macro {
	
	public static final char MACRO_DEFINITION_PERIMETER = TextParserUtil.ENCLOSE_MACRO;
	
	private String name;
	private String[] parameters;
	private char[] separators;
	private String definition;
	
	private Macro(String name_arg, String[] parameters_arg, char[] separators_arg, String definition_arg) {
		logAssert(name_arg!=null && !name_arg.trim().isEmpty());
		this.name = name_arg;
		this.parameters = parameters_arg != null && parameters_arg.length > 0 ? parameters_arg : null;
		this.separators = separators_arg != null && separators_arg.length > 0 ? separators_arg : null;
		this.definition = definition_arg != null && !definition_arg.isEmpty() ? definition_arg : null;
		
		if(this.separators != null && this.parameters.length > 1)
			logAssert(this.separators != null && this.separators.length == this.parameters.length -1);
		if(this.separators != null)
			logAssert(this.parameters != null && this.separators.length == this.parameters.length -1);
	}
	
	public String getName() {
		return name;
	}
	
	public char[] getSeparators() {
		return separators;
	}
	
	public int getParameterCount() {
		return parameters == null ? 0 : parameters.length;
	}
	
	public void pushDefinition(CharStack stack, String[] parameters, int line, int col) {
		logAssert(getParameterCount() == (parameters == null ? 0 : parameters.length));
		
		if(parameters != null && parameters.length > 0) {
			MacroMap map = new MacroMap();
			for(int i = 0; i < parameters.length; i++) {
				Macro macro = makeMacroFromString(this.parameters[i]);
				macro.definition = parameters[i]; //Surprise definition!
				if(!map.tryAddMacro(macro)) {
					log(ERROR, "Two macro parameters in the macro '%s' have the same name: '%s'", name, macro.getName());
				}
			}
			stack.pushMacroMap(map);
			stack.pushMacroMapPopCommand(line, col);
		}
		
		for(int i = definition.length()-1; i >= 0; i--) {
			stack.pushBufferedChar(definition.charAt(i), line, col);
		}
	}
	
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		
		builder.append(name);
		builder.append("<");
		
		for(int i = 0; parameters != null && i < parameters.length; i++) {
			builder.append(parameters[i]);
			if(separators != null && i < separators.length)
				builder.append(separators[i]).append(" ");
		}
		builder.append(String.format("> %s", definition));
		
		return builder.toString();
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
		
		//=========================== Now at START OF NAME ================
		
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
		
		//=============================== LOOK FOR PARAMETERS ===================
		
		ArrayList<String> parameters = new ArrayList<>();
		StringBuilder separators = new StringBuilder();
		if(TextParserUtil.isStartOfMacroParameters(text.charAt(index))) { //We've got parameters!
			boolean lookingForParameter = true;
			int startOfParam = -1;
			
			index++;
			
			int scope = 1;
			
			while(index < text.length()) {
				char c = text.charAt(index);
				
				if(startOfParam<0 && scope <= 0)
					break;
				
				if(TextParserUtil.isStartOfMacroParameters(c))
					scope++;
				
				if(lookingForParameter && !TextParserUtil.isNormalWhitespace(c) && !TextParserUtil.isEndOfMacroParameters(c)) {
					if(!TextParserUtil.isStartOfIdentifier(c)) {
						log(ERROR, "A macro parameter must start with a letter or underscore, not '%c'", c);
						return null;
					}
					startOfParam = index;
					lookingForParameter = false;
				}
				else if(startOfParam >= 0 && scope == 1 && !TextParserUtil.isIdentifierChar(c)) {
					parameters.add(text.substring(startOfParam,index));
					startOfParam = -1;
				}
				
				if(TextParserUtil.isEndOfMacroParameters(c))
					scope--;
				
				if(startOfParam < 0 && !lookingForParameter) { // lookingForParameter has to be false, but good code is self-descriptive
					if(scope <= 0) {
						continue;
					}
					else if(TextParserUtil.isLegalMacroParameterSeparator(c)) {
						separators.append(c);
						lookingForParameter = true;
					} else if(!TextParserUtil.isNormalWhitespace(c)) {
						if(TextParserUtil.isStartOfMacroParameters(c))
							log(ERROR, "Found the char '%c', the start of a new macro parameter, before a separator was found!", c);
						else
							log(ERROR, "The special char '%c' was found in a macro parameter list, but can't be a separator", c);
						return null;
					}
				}
				
				index++;
			}
			index++;
		}
		
		//Parameters and separators found
		
		while(index < text.length() && TextParserUtil.isNormalWhitespace(text.charAt(index))) //Skip whitespaces
			index++;
		
		if(index >= text.length()) //A macro with a parameter list, but no definition... Why?
			return new Macro(macroName, parameters.toArray(new String[parameters.size()]), separators.toString().toCharArray(), null);
		
		//================================= LOOK FOR DEFINITION =========================
		
		char definitionEnd = TextParserUtil.END_OF_LINE;
		if(text.charAt(index) == MACRO_DEFINITION_PERIMETER) {
			index++;
			definitionEnd = MACRO_DEFINITION_PERIMETER;
		}
		
		int definitionStart = index;
		
		//TODO: Skip comments (important)
		while(true) {
			if(index >= text.length()) {
				if(TextParserUtil.isNewlineChar(definitionEnd))
					log(WARNING, "A macro definition supposedly ending with a %c stopped before it occured", definitionEnd);
			}
			if(text.charAt(index) == definitionEnd)
				break;
			index++;
		}
		
		String definition = text.substring(definitionStart, index);
		
		return new Macro(macroName, parameters.toArray(new String[parameters.size()]), separators.toString().toCharArray(), definition);
	}
	
	public static interface CharStack {
		 void pushBufferedChar(char c, int line, int col);
		 
		 void pushMacroMap(MacroMap map);
		 
		 void pushMacroMapPopCommand(int line, int col);
	}
}
