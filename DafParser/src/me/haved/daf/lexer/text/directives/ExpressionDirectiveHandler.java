package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class ExpressionDirectiveHandler{
	public static final String EXPRESSION_DIRECIVE_START = "(";
	
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		if(!text.equals(EXPRESSION_DIRECIVE_START)) {
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		}
		
		pp.giveUpControlTo(new ExpressionPPController(line, col));
		
		return DirectiveHandler.HANDLED;
	}
}
