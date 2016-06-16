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
	
	private boolean parameterList = false;
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
	
	int scope = 0;
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler) {
		
		char c = pp.getCurrentChar();
		if(!parameterList) {
			if(TextParserUtil.isStartOfMacroParameters(c)) {
				parameterList = true;
				paramLookingAt = 0;
				scope = 1;
				buffer.setLength(0);
				updateNextSeparator();
				return false;
			}
			else if(TextParserUtil.isNormalWhitespace(c))
				buffer.append(c);
			else { //We skipped chars but no parameter list was to be seen
				pushMacroDefinition(inputHandler, line, col);
				inputHandler.pushMultipleChars(buffer.toString(), line, col);
				pp.popBackControll();
			}
		}
		
		if(TextParserUtil.isStartOfIdentifier(c))
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
		nextSep = paramLookingAt < macro.getSeparators().length ? macro.getSeparators()[paramLookingAt] : TextParserUtil.END_OF_MACRO_PARAMETER;
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
		} else
			macro.pushDefinition(handler, parameters, line, col);
	}
	
	@Override
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp,
			InputHandler inputHandler) {
		return true; //Allow everything to happen
	}

	@Override
	public String getName() {
		return "Macro Evaluation PreProcController";
	}

}