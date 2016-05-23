package me.haved.daf.lexer.text;

import java.util.ArrayList;

import static me.haved.daf.LogHelper.*;

public class MacroMap {
	private ArrayList<Macro> macros;
	
	public MacroMap() {
		macros = new ArrayList<>();
	}
	
	public boolean tryAddMacro(String text) {
		Macro newMacro = Macro.getMacroFromString(text); //Checking if text is bad is done in here!
		if(newMacro == null) //getMacroFromString will in this case already have written error messages
		{
			log(MESSAGE, "The macro text in question: \"%s\"", text);
			return false;
		}
		
		return tryAddMacro(newMacro);
	}
	
	public boolean tryAddMacro(Macro macro) {
		if(macro == null)
			log(ASSERTION_FAILED, "Macro given to tryAddMacro was null");
		Macro oldMacro = getMacro(macro.getMacroName());
		if(oldMacro != null) {
			log(ERROR, "Macro by name of '%s' was already defined!", oldMacro.getMacroName());
			return false;
		}
		
		macros.add(macro);
		return true;
	}
	
	public Macro getMacro(String name) {
		for(Macro macro:macros) {
			if(name.equals(macro.getMacroName()))
				return macro;
		}
		return null;
	}
	
	public String toString() {
		StringBuilder builder = new StringBuilder("(MacroMap, ");
		for(Macro macro:macros) {
			builder.append(String.format("\n %s ", macro.toString()));
		}
		
		return builder.append(")").toString();
	}
}