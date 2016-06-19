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
			putElmOnStack(inputHandler, inQuotes);
			inQuotes = !inQuotes;
		} else if(!inQuotes && TextParserUtil.isAnyWhitespace(c)) {
			putElmOnStack(inputHandler, false);
		} else if(!inQuotes && c == EXPRESSION_DIRECTIVE_END) {
			putElmOnStack(inputHandler, false);
			if(stack.size()>0) {
				//log(VERBOSE, "Expression evaluated to: %s", stack.peek());
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

	private void putElmOnStack(InputHandler handler, boolean forced) {
		if(currentElm.length()==0 && !forced) {
			return;
		}
		
		String elm = currentElm.toString();
		
		if(!handleSpecialElement(elm, handler))
			stack.push(elm);
		currentElm.setLength(0);
	}
	
	/**
	 * @param element - the string
	 * @param handler - an inputHandler to get error location data from
	 * @return true if the element was special and should not be put on the stack itself
	 */
	private boolean handleSpecialElement(String elm, InputHandler handler) {
		for(Operator op:Operator.OPERATORS) {
			if(elm.equals(op.getName())) {
				if(stack.size() < op.getParamCount()) {
					log(handler.getFile(), handler.getInputLine(), handler.getInputCol()-elm.length(), ERROR, 
							"The operator %s requires %d elements to be on the stack, not just %d", elm, op.getParamCount(), stack.size());
					log(INFO, "The arguments already present were:");
					for(int i = 0; i < stack.size(); i++) {
						log(INFO, stack.get(i));
					}
					for(int i = stack.size(); i < op.getParamCount(); i++) {
						stack.push(""); //Fill it with enough
					}
				}
				
				Object[] params = new Object[op.getParamCount()];
				
				for(int i = params.length -1; i >= 0; i--) {
					String stackElm = stack.pop();
					if(op.canTakeInt(i)) {
						try {
							params[i] = Integer.parseInt(stackElm);
							continue;
						} catch(Exception e) {} //I don't like throwing exceptions on purpose
					}
					if(op.canTakeString(i))
						params[i] = stackElm;
					else {
						log(handler.getFile(), handler.getInputLine(), handler.getInputCol(), ERROR, 
								"Parameter #%d of the %s operator must be an integer! Defaulting to 0", i, op.getName());
						params[i] = 0;//new Integer(0);
					}
				}
				
				String result = op.getDoer().doOperator(params);
				
				logAssert(result != null);
				
				String warning = Operator.parseWarning(result);
				if(warning != null) {
					result = warning;
					log(handler.getFile(), handler.getInputCol(), handler.getInputLine(), WARNING, 
							"Previous operator fault made the %s operator output: %s", op.getName(), result);
				}
				
				stack.push(result);
				
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
