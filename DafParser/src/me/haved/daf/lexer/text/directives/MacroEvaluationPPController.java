package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.Macro;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

import static me.haved.daf.LogHelper.*;

public class MacroEvaluationPPController implements PreProcessorController {

	private int line, col;
	
	private Macro macro;
	private String[] parameters;
	
	int scope = 0;
	private int paramLookingAt = 0;
	
	private StringBuilder buffer;
	
	private char nextSep;
	
	public MacroEvaluationPPController(Macro macro, int line, int col) {
		this.macro = macro;
		this.line = line;
		this.col = col;
		this.parameters = new String[macro.getParameterCount()];
		this.buffer = new StringBuilder();
	}
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler) {
		char c = pp.getCurrentChar();
		if(scope == 0) {
			if(TextParserUtil.isStartOfMacroParameters(c)) {
				paramLookingAt = 0;
				scope = 1;
				buffer.setLength(0);
				updateNextSeparator();
			}
			else if(TextParserUtil.isNormalWhitespace(c))
				buffer.append(c);
			else { //We skipped chars but no parameter list was to be seen
				inputHandler.pushCurrentChar();
				inputHandler.pushMultipleChars(buffer.toString(), line, col);
				pushMacroDefinition(inputHandler, line, col);
				pp.popBackControll();
				
			}
			return false;
		}
		
		if(TextParserUtil.isStartOfMacroParameters(c))
			scope++;
		
		if(paramLookingAt < parameters.length) {
			if(c == nextSep && scope == 1) {
				parameters[paramLookingAt] = buffer.toString();
				buffer.setLength(0);
				paramLookingAt++;
				updateNextSeparator();
			}
			else
				buffer.append(c);
		}
		
		if(TextParserUtil.isEndOfMacroParameters(c))
			scope--;
		
		if(scope == 0) {
			pushMacroDefinition(inputHandler, line, col);
			pp.popBackControll();
		}
			
		return false;
	}

	private void updateNextSeparator() {
		nextSep = paramLookingAt < macro.getSeparatorCount() ? macro.getSeparators()[paramLookingAt] : TextParserUtil.END_OF_MACRO_PARAMETER;
	}
	
	private void pushMacroDefinition(InputHandler handler, int line, int col) {
		if(paramLookingAt != parameters.length) {
			log(handler.getFile(), line, col, ERROR, "The macro %s requires %d parameters, but %d were given!", 
					macro.getName(), parameters.length, paramLookingAt);
			log(INFO, "The macro signature: %s\n", macro.getSignature());
			log(VERBOSE, "Parameters given:");
			for(int i = 0; i < paramLookingAt; i++) {
				log(VERBOSE, parameters[i]);
			}
			log(VERBOSE, "End of list");
		} else if(macro.hasDefinition()) //No point in anything really, if there is no definition
			macro.pushDefinition(handler, parameters, line, col);
	}
	
	@Override
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp,
			InputHandler inputHandler) {
		return true; //Allow everything to happen
	}
	
	@Override
	public boolean lookForDirectives() {
		return scope > 0;
	}

	@Override
	public String getName() {
		return "Macro Evaluation PreProcController";
	}

}
