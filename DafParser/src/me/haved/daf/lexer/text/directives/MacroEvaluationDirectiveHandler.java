package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.Macro;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;


public class MacroEvaluationDirectiveHandler {
	
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		Macro macro = inputHandler.getMacro(text);
		if(macro == null)
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		
		//We found a macro, guys!
		pp.giveUpControlTo(new MacroEvaluationPPController(macro, line, col));
		
		return DirectiveHandler.HANDLED;
	}
}
