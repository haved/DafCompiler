package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;

public interface DirectiveHandler {
	
	public static int CANT_HANLDE_DIRECTIVE = 0; //Try some other directive handler
	public static int HANDLED = 1; //After this is returned, the next char to be read is in the buffer or file!
	public static int HANDLING_ERROR = -1;
	
	/**
	 * The InputHandler must be advanced once before you're past the directive name
	 * 
	 * @param text is the name of the directive in question
	 * @param line is the line number of the pound symbol
	 * @param col is the lien column of the pound symbol
	 * @param pp is the reference to the preprocessors input handler
	 * @return CANT_HANDLE_DIRECTIVE if the name was wrong for this exact supplier. HANDLED if everything went a'ok. HANDLING_ERROR if errors occurred
	 */
	int handleDirective(String text, int line, int col, PreProcessor pp, PreProcessor.InputHandler inputHandler);
}
