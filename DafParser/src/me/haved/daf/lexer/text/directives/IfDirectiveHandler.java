package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class IfDirectiveHandler {

	public static final String IF_DIRECTIVE = "if";
	
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		if(!text.equals(IF_DIRECTIVE)) {
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		}
		
		pp.giveUpControlTo(new IfPPController(line, col));
		
		return DirectiveHandler.HANDLED;
	}
}
