package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.Macro;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

import static me.haved.daf.LogHelper.*;

public class MacroEvaluationDirectiveHandler {
	
	public static int handleDirective(String text, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		Macro macro = inputHandler.getMacro(text);
		if(macro == null)
			return DirectiveHandler.CANT_HANLDE_DIRECTIVE;
		//We found a macro, guys!
		
		inputHandler.advanceInput();
		boolean hasParameters = skipSpaces(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol(), inputHandler);
		//If parameters == true, current char is the '<'
		//If not, the next char on the stack is the next one in the file
		
		if(hasParameters == false) {
			if(macro.getParameterCount() != 0) {
				log(inputHandler.getFile(), line, col, ERROR, "The macro '%s' takes %d parameters, but none were given!", 
						macro.getName(), macro.getParameterCount());
				return DirectiveHandler.HANDLING_ERROR;
			}
			macro.pushDefinition(inputHandler, null, line, col);
		} else {
			char[] separators = macro.getSeparators();
			int currentSeparator = 0;
			int separatorCount = separators == null ? 0 : separators.length;
			String[] parameters = new String[macro.getParameterCount()];
			int currentParameter = 0;
			
			StringBuilder parameter = new StringBuilder();
			
			int scope = 1;
			
			while(true) {
				inputHandler.advanceInput();
				char c = inputHandler.getInputChar();
				
				if(TextParserUtil.isNewlineChar(c)) {
					log(inputHandler.getFile(), line, col, ERROR, "Newline or EOF found during macro parameter list!");
					return DirectiveHandler.HANDLING_ERROR;
				}
				
				if(TextParserUtil.isStartOfMacroParameters(c))
					scope++;
				else if(TextParserUtil.isEndOfMacroParameters(c))
					scope--;
				
				if(scope == 0 || (scope == 1 && currentSeparator < separatorCount && separators[currentSeparator] == c)) {
					if(parameters.length == 0) {
						if(!parameter.toString().trim().isEmpty()) {
							log(inputHandler.getFile(), line, col, ERROR, "The macro '%s' takes no parameters, yet '%s' was given!", 
									macro.getName(), parameter.toString());
							return DirectiveHandler.HANDLING_ERROR;
						}
						if(scope == 0)
							break;
					} else {
						logAssert(currentParameter < parameters.length);
						parameters[currentParameter] = parameter.toString();
						parameter.setLength(0);
						currentParameter++;
						currentSeparator++;
						if(scope == 0)
							break;
						continue;
					}
				}
				
				parameter.append(c);
			}
			//Now currently at the final '>', so the next char on the stack is properly aligned
			
			if(currentParameter < parameters.length) {
				log(inputHandler.getFile(), line, col, ERROR, "The macro '%s' takes %d parameters, but only %d were given!",
						macro.getName(), parameters.length, currentParameter);
				return DirectiveHandler.HANDLING_ERROR;
			}
			
			macro.pushDefinition(inputHandler, parameters, line, col);
		}
		
		return DirectiveHandler.HANDLED;
	}
	
	private static boolean skipSpaces(char c, int line, int col, PreProcessor.InputHandler pp) {
		if(TextParserUtil.isNormalWhitespace(c)) {
			pp.advanceInput();
			if(!skipSpaces(pp.getInputChar(), pp.getInputLine(), pp.getInputCol(), pp)) {
				pp.pushBufferedChar(c, line, col); //If it turns out we we're not supposed to skip the char, add it back
				return false;
			}
			return true;
		}
		
		if(TextParserUtil.isStartOfMacroParameters(c))
			return true;
		
		pp.pushBufferedChar(c, line, col);
		return false;
	}
}
