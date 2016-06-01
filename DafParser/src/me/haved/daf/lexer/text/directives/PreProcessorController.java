package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public interface PreProcessorController {

	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler);
	
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp, InputHandler inputHandler);
	
	public String getName();
}
