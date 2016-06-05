package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class MacroDirectiveHandler {
	
	public static final String DIRECTIVE_NAME = "macro";
	 
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		if(!text.equals(DIRECTIVE_NAME))
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		
		pp.giveUpControlTo(new MacroPPController(line, col));
		
		return DirectiveHandler.HANDLED;
		
		/*StringBuilder builder = new StringBuilder();
		
		boolean startFound = false;
		
		if(TextParserUtil.isAnyWhitespace(inputHandler.getInputChar()))
			inputHandler.advanceInput(); //Skip the first whitespace
		
		while(true) {
			if(!inputHandler.advanceInput()) {
				log(inputHandler.getFile(), inputHandler.getInputLine(), inputHandler.getInputCol(), ERROR, "The macro definition wasn't done by the end of the file!");
				log(SUGGESTION, "Have you forgotten a '%c' to end the multi-line definition?", Macro.MACRO_DEFINITION_PERIMETER);
				break;
			}
			
			char c = inputHandler.getInputChar();
			
			builder.append(c);
			
			if(c == Macro.MACRO_DEFINITION_PERIMETER) {
				if(startFound)
					break;
				startFound = true;
			}
			else if(TextParserUtil.isNewlineChar(c) & !startFound)
				break;
		}
		
		Macro macro = Macro.makeMacroFromString(builder.toString());
		if(macro == null) {
			log(inputHandler.getFile(), inputHandler.getInputLine(), inputHandler.getInputCol(), ERROR, "Aborting macro due to previous errors");
			return DirectiveHandler.HANDLING_ERROR;
		}
		
		if(!inputHandler.addMacro(macro))
			log(inputHandler.getFile(), line, col, SUGGESTION, "Change the name of the macro if you don't want to override");
		*/
	}
}
