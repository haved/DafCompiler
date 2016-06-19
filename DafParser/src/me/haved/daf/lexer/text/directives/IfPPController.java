package me.haved.daf.lexer.text.directives;

import static me.haved.daf.LogHelper.*;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;
import me.haved.daf.lexer.text.TextParserUtil;

public class IfPPController implements PreProcessorController {
	
	public static final int FALSE_INT = 0;
	public static final int TRUE_INT = 1;
	public static final String FALSE_STRING = "0";
	public static final String TRUE_STRING = "1";
	
	public static final String THEN_DIRECTIVE = "then";
	public static final String ELSE_DIRECTIVE = "else";
	public static final String ENDIF_DIRECTIVE = "endif";
	
	private final int line, col;
	
	private StringBuilder expression;
	private boolean readingExpression;
	private boolean ifFulfilled;

	public IfPPController(int line, int col) {
		this.line = line;
		this.col = col;
		expression = new StringBuilder();
		readingExpression = true;
		ifFulfilled = false;
	}
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler) {
		if(readingExpression) {
			expression.append(pp.getCurrentChar());
			return false;
		}
		logAssert(ifFulfilled);
		return true;
	}

	@Override
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp, InputHandler inputHandler) {	
		if(directiveText.equals(THEN_DIRECTIVE)) {
			if(!readingExpression) {
				log(pp.getFile(), line, col, ERROR, "A rouge #%s directive was found within another if directive. What to do?", THEN_DIRECTIVE);
				return true;
			}
			readingExpression = false;
			
			boolean expressionValue = false;
			String trimmed = expression.toString().trim();
			try {
				expressionValue = evaluateIntToBoolean(Integer.parseInt(trimmed));
			} catch(Exception e) {
				log(inputHandler.getFile(), this.line, this.col, ERROR, 
						"The condition of the #if directive was neither '%s' nor '%s', "
						+ "but '%s'. Using false by default", TRUE_STRING, FALSE_STRING, trimmed);
			}
			
			if(!expressionValue) {
				logAssert(!ifFulfilled);
				skipStatement(pp, inputHandler);
			}
			else
				ifFulfilled = true;
			
			return false;
		}
		else if(directiveText.equals(ENDIF_DIRECTIVE)) {
			pp.popBackControll();
			if(readingExpression) {
				log(pp.getFile(), line, col, ERROR, "An #%s was found in an if-statement before #then occured!", ENDIF_DIRECTIVE);
				return true;
			}
			return false;
		} else if(directiveText.equals(ELSE_DIRECTIVE)) {
			if(readingExpression) {
				log(pp.getFile(), line, col, ERROR, "An #%s was found in an if-statement before #then occured!", ELSE_DIRECTIVE);
				return true;
			}
			logAssert(ifFulfilled);
			ifFulfilled = false;
			skipStatement(pp, inputHandler);
			return false;
		}
		
		return true;
	}
	
	private void skipStatement(PreProcessor pp, InputHandler inputHandler) {
		int scope = 1;
		
		while(scope > 0) {
			if(!inputHandler.advanceInput()) {
				log(pp.getFile(), ERROR, "#if evaluating to false starting at (%d:%d) never ended! Skipped %d lines!", line, col, 
						inputHandler.getInputLine()-line);
			}
			if(TextParserUtil.isPoundSymbol(inputHandler.getInputChar())) {
				inputHandler.advanceInput();
				String directive = PreProcessor.pickUpPreProcDirective(inputHandler);
				if(directive.equals(IfDirectiveHandler.IF_DIRECTIVE))
					scope++;
				else if(directive.equals(ENDIF_DIRECTIVE))
					scope--;
				else if(directive.equals(ELSE_DIRECTIVE) && scope == 1) {
					logAssert(!ifFulfilled);
					ifFulfilled = true;
					return;
				}
			}
		}
		
		logAssert(scope==0);
		pp.popBackControll();
	}

	@Override
	public String getName() {
		return "If-statement controller";
	}
	
	public static boolean evaluateIntToBoolean(int i) {
		return i == TRUE_INT;
	}
}
