package me.haved.daf.lexer.text;

import java.util.ArrayList;

import static me.haved.daf.LogHelper.*;

public class MacroMap {
	
	private ArrayList<Macro> macros;
	
	public MacroMap() {
		macros = new ArrayList<>();
	}
	
	public boolean tryAddMacro(String text) {
		if(text==null || text.trim().isEmpty()) {
			log(ERROR, "Macro text passed was empty!");
			return false;
		}
		
		Macro macro = Macro.makeMacroFromString(text);
		if(macro == null) {
			log(ERROR, "Macro text in question: '%s'", text);
			return false;
		}
		
		return tryAddMacro(macro);
	}

	public boolean tryAddMacro(Macro macro) {
		if(macro == null)
			return false;
		
		boolean returnVal = true;
		Macro macro2 = getMacro(macro.getName());
		if(macro2 != null) {
			log(WARNING, "A macro with the name %s already exists!", macro.getName());
			returnVal = false;
			macros.remove(macro2);
		}
		
		macros.add(macro);
		return returnVal;
	}
	
	public Macro getMacro(String name) {
		for(Macro macro:macros) {
			if(macro.getName().equals(name))
				return macro;
		}
		return null;
	}
}
