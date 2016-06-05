package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class MacroDirectiveHandler {
	
	public static final String DIRECTIVE_NAME = "macro";
	 
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		if(!text.equals(DIRECTIVE_NAME))
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		
		pp.giveUpControlTo(new MacroPPController(line, col));
		
		return DirectiveHandler.HANDLED;
	}
}
