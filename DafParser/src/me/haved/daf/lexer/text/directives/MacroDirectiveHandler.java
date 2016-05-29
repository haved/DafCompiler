package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.Macro;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextParserUtil;

import static me.haved.daf.LogHelper.*;

public class MacroDirectiveHandler {
	
	public static final String DIRECTIVE_NAME = "macro";
	 
	public static int handleDirective(String text, int line, int col, PreProcessor.InputHandler pp) {
		if(!text.equals(DIRECTIVE_NAME))
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		
		StringBuilder builder = new StringBuilder();
		
		boolean startFound = false;
		
		if(TextParserUtil.isAnyWhitespace(pp.getInputChar()))
			pp.advanceInput(); //Skip the first whitespace
		
		while(true) {
			if(!pp.advanceInput()) {
				log(pp.getFile(), pp.getInputLine(), pp.getInputCol(), ERROR, "The macro definition wasn't done by the end of the file!");
				log(SUGGESTION, "Have you forgotten a '%c' to end the multi-line definition?", Macro.MACRO_DEFINITION_PERIMETER);
				break;
			}
			
			char c = pp.getInputChar();
			
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
			log(pp.getFile(), pp.getInputLine(), pp.getInputCol(), ERROR, "Aborting macro due to previous errors");
			return DirectiveHandler.HANDLING_ERROR;
		}
		
		pp.addMacro(macro);
			
		return DirectiveHandler.HANDLED;
	}
}
