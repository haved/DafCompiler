package me.haved.daf.lexer;

// #macro Ball true
// #macro Bull<> {false}
// #macro Func <#param> {#param}
// 

import static me.haved.daf.LogHelper.*;

public class Macro {
	private String name;
	private String[] parameters;
	private String definition;
	
	private Macro(String name, String[] parameters, String definition) {
		this.name = name;
		this.parameters = parameters;
		this.definition = definition;
	}
	
	public String getMacroName() {
		return name;
	}
	
	public String[] getMacroParameters() {
		return parameters;
	}
	
	public String getMacroValue() {
		return definition;
	}
	
	
	public static Macro getMacroFromString(String text) {
		if(text == null || (text = text.trim()).length() < 1) {
			log(ERROR, "Trying to add a macro without any text");
			return null;
		}
		
		return new Macro(text, null, null);
	}
}
