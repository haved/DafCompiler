package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class IfDirectiveHandler implements DirectiveHandler {

	@Override
	public int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		return 0;
	}
}
