package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;

public interface DirectiveHandler {
	
	public static int CANT_HANLDE_DIRECTIVE = 0; //Try some other directive handler
	public static int HANDLED = 1; //After this is returned, the next char to be read is in the buffer or file!
	public static int HANDLING_ERROR = -1;
	
	int handleDirective(String text, PreProcessor.InternalPreProcessor pp);
}
