package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;

public interface PreProcessorController {

	public boolean allowAdvanceToReturn(PreProcessor pp);
	
	public boolean allowDirectiveToHappen(PreProcessor pp, String directiveText);
}
