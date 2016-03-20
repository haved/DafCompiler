package me.haved.daf.args;

import static me.haved.daf.LogHelper.*;

public class MacroOption implements CommandOption {
	
	private AddMacroCall call;
	
	public MacroOption(AddMacroCall call) {
		this.call = call;
	}
	
	@Override
	public int parseOption(String[] args, int pos) {
		if(args[pos].equals("-m")) {
			if(pos+2 >= args.length)
				log(FATAL_ERROR, "-m needs two options following it!");
			
			println("Macro -m saying that: '%s'->'%s'", args[pos+1], args[pos+2]);
			
			if(!call.defineMacro(args[pos+1], args[pos+2]))
				log(FATAL_ERROR, "-m was given illegal options for a macro definition!");
			
			return 3; // the -m as well as macro 
		}
		
		return 0;
	}

	@Override
	public String getName() {
		return "-m <name> [{] <value> [}] ";
	}

	@Override
	public String getDescription() {
		return "Defines a macro in parsing. The name is an identifier, and the value can be anything. If there are spaces in the value, use { and }";
	}
	
	public static interface AddMacroCall {
		public boolean defineMacro(String name, String contents);
	}
}
