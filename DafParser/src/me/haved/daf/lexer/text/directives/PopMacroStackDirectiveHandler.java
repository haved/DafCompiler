package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class PopMacroStackDirectiveHandler {
	
	public static final String POP_MACRO_STACK_DIRECTIVE = "PoP_mAcRo_StAcK";
	
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		
		if(!text.equals(POP_MACRO_STACK_DIRECTIVE))
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		
		inputHandler.popMacroMap();
		
		return DirectiveHandler.HANDLED;
	}
}
