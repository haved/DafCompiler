package me.haved.daf.lexer.text.directives;

import java.util.Stack;

import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;
import me.haved.daf.lexer.text.TextParserUtil;

import static me.haved.daf.LogHelper.*;

public class ExpressionPPController implements PreProcessorController {

	public static final char EXPRESSION_DIRECTIVE_END = ')';
	public static final String EXPRESSION_DIRECIVE_END = Character.toString(EXPRESSION_DIRECTIVE_END);
	
	private int line, col;
	
	private Stack<String> stack;
	private StringBuilder currentElm;
	
	public ExpressionPPController(int line, int col) {
		this.line = line;
		this.col = col;
		
		this.stack = new Stack<>();
		currentElm = new StringBuilder();
	}
	
	boolean inQuotes = false;
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler) {
		char c = inputHandler.getInputChar();
		
		if(TextParserUtil.isDoubleQuoteChar(c)) {
			inQuotes = !inQuotes;
			putElmOnStack(inputHandler); //Only if elm has got content
		} else if(!inQuotes && TextParserUtil.isAnyWhitespace(c)) {
			putElmOnStack(inputHandler);
		} else if(!inQuotes && c == EXPRESSION_DIRECTIVE_END) {
			putElmOnStack(inputHandler);
			if(stack.size()>0) {
				inputHandler.pushMultipleChars(stack.pop(), this.line, this.col);
			}
			pp.popBackControll();
		} else {
			currentElm.append(c);
		}
		
		return false;
	}

	@Override
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp, InputHandler inputHandler) {	
		return true;
	}

	private void putElmOnStack(InputHandler handler) {
		if(currentElm.length()==0) {
			return;
		}
		
		String elm = currentElm.toString();
		
		if(!handleSpecialElement(elm, handler))
			stack.push(elm);
		currentElm.setLength(0);
	}
	
	private boolean handleSpecialElement(String elm, InputHandler handler) {
		for(Operator op:Operator.operators) {
			if(elm.equals(op.getName())) {
				if(stack.size() < op.getParamCount()) {
					log(handler.getFile(), handler.getInputChar(), handler.getInputCol()-elm.length(), ERROR, 
							"The operator %s' requires %d elements to be on the stack, not just %d", elm, op.getParamCount(), stack.size());
					for(int i = stack.size(); i < op.getParamCount(); i++) {
						stack.push(""); //Fill it with enough
					}
				}
				
				boolean ints = op.canTakeInts();
				Object[] params = new Object[op.getParamCount()];
				
				for(int i = params.length -1; i >= 0; i--) {
					String stackElm = stack.pop();
					if(ints) {
						try {
							params[i] = Integer.parseInt(stackElm);
							continue;
						} catch(Exception e) {} //I don't like throwing exceptions on purpose
					}
					params[i] = stackElm;
				}
				
				stack.push(op.getDoer().doOperator(params));
				
				return true;
			}
		}
		return false;
	}
	
	@Override
	public String getName() {
		return "Expression-parser controller";
	}
}
