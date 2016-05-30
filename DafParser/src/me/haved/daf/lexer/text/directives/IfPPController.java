package me.haved.daf.lexer.text.directives;

import static me.haved.daf.LogHelper.ERROR;
import static me.haved.daf.LogHelper.log;

import me.haved.daf.lexer.text.PreProcessor;

public class IfPPController implements PreProcessorController {
	
	public static final String THEN_DIRECTIVE = "then";
	public static final String ENDIF_DIRECTIVE = "endif";
	
	private StringBuilder expression;
	private boolean readingExpression;
	private boolean ifFulfilled;
	
	public IfPPController() {
		expression = new StringBuilder();
		readingExpression = true;
		ifFulfilled = false;
	}
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp) {
		if(readingExpression)
			expression.append(pp.getCurrentChar());
		return ifFulfilled;
	}

	@Override
	public boolean allowDirectiveToHappen(PreProcessor pp, String directiveText) {	
		if(directiveText.equals(THEN_DIRECTIVE)) {
			if(!readingExpression) {
				log(ERROR, "A rouge #then directive was found within another if directive. What to do?");
				return true;
			}
			readingExpression = false;
			ifFulfilled = expression.toString().trim().equals("1");
			return false;
		}
		if(directiveText.equals(ENDIF_DIRECTIVE)) {
			pp.popBackControll();
			return false;
		}
		return true;
	}
}
