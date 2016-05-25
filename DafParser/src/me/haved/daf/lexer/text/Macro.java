package me.haved.daf.lexer.text;

import static me.haved.daf.LogHelper.*;

public class Macro {
	
	private String name;
	private String[] parameters;
	private char[] separators;
	private String definition;
	
	private Macro(String name, String[] parameters, char[] separators, String definition) {
		logAssert(name==null || name.trim().isEmpty());
		this.name = name;
		this.parameters = parameters != null && parameters.length > 0 ? parameters : null;
		this.separators = separators != null && separators.length > 0 ? separators : null;
		this.definition = definition != null && !definition.isEmpty() ? definition : null;
		
		if(parameters != null && parameters.length > 1)
			logAssert(separators != null && separators.length == parameters.length -1);
		if(separators != null)
			logAssert(parameters != null && separators.length == parameters.length -1);
	}
	
	public String getName() {
		return name;
	}
	
	public static Macro makeMacroFrom() {
		return null;
	}
}
