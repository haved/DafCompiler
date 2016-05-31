package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class IfDirectiveHandler implements DirectiveHandler, PreProcessorController {

	public static final String IF_DIRECTIVE = "if";
	public static final String ENDIF_DIRECTIVE = "endif";
	
	@Override
	public int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		if(!text.equals(IF_DIRECTIVE)) {
			return CANT_HANLDE_DIRECTIVE;
		}
		
		pp.giveUpControlTo(this);
		
		return HANDLED;
	}

	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp) {
		return false;
	}

	@Override
	public boolean allowDirectiveToHappen(PreProcessor pp, String directiveText) {
		if(directiveText.equals(ENDIF_DIRECTIVE)) {
			pp.popBackControll();
			return false;
		}
		return true;
	}
}
