package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;

public class MacroDirectiveHandler {
	
	public static final String DIRECTIVE_NAME = "macro";
	
	public static int handleDirective(String text, PreProcessor.InternalPreProcessor pp) {
		if(!text.equals(DIRECTIVE_NAME))
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		return DirectiveHandler.HANDLED;
	}
}
