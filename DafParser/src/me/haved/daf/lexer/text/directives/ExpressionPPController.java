package me.haved.daf.lexer.text.directives;

import java.util.Stack;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;
import me.haved.daf.lexer.text.TextParserUtil;

public class ExpressionPPController implements PreProcessorController {

	public static final String EXPRESSION_DIRECIVE_END = ")";
	
	private int line, col;
	
	private Stack<String> stack;
	private StringBuilder currentElm;
	
	public ExpressionPPController(int line, int col) {
		this.line = line;
		this.col = col;
		
		this.stack = new Stack<>();
	}
	
	boolean inQuotes = false;
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler) {
		char c = inputHandler.getInputChar();
		
		if(TextParserUtil.isDoubleQuoteChar(c)) {
			inQuotes = !inQuotes;
			putElmOnStack(); //Only if elm has got content
		} else if(!inQuotes && TextParserUtil.isNormalWhitespace(c)) {
			putElmOnStack();
		} else {
			currentElm.append(c);
		}
		
		return false;
	}

	@Override
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp, InputHandler inputHandler) {
		
		if(directiveText == EXPRESSION_DIRECIVE_END) {
			putElmOnStack();
			return false;
		}
		
		return true;
	}

	private void putElmOnStack() {
		if(currentElm.length()==0) {
			return;
		}
		
		String elm = currentElm.toString();
		
		if(!handleSpecialElement(elm))
			stack.push(elm);
		currentElm.setLength(0);
	}
	
	private boolean handleSpecialElement(String elm) {
		if(elm.length() < 2) {
			
		}
		return false;
	}

	private static final Operator[] operators = {};
	
	class Operator {
		
	}
	
	@Override
	public String getName() {
		return "Expression-parser controller";
	}

}
