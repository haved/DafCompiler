package me.haved.daf.lexer.text.directives;

import java.util.Stack;

import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class ExpressionAction {
	
	public static final ExpressionAction[] ACTIONS = {
			new ExpressionAction("swap", 2, (stack, input)->{String s = stack.pop(); String s2 = stack.pop(); stack.push(s); stack.push(s2);}),
			new ExpressionAction("dup", 1, (stack, input)->{stack.push(stack.peek());}),
			new ExpressionAction("lineNum", 0, (stack, input)->stack.push(Integer.toString(input.getInputLine()))),
			new ExpressionAction("colNum", 0, (stack, input)->stack.push(Integer.toString(input.getInputCol()))),
			new ExpressionAction("exists", 1, 
					(stack, input)->stack.push(input.getMacro(stack.peek())!= null ? IfPPController.TRUE_STRING : IfPPController.FALSE_STRING)),
			new ExpressionAction("macroStack", 0, (stack, input)->stack.push(Integer.toString(input.getMacroStack().size()))),
			new ExpressionAction("pop", 0, (stack, input)->stack.pop())
	};
	
	//Xswap, controllerStack, eval
	private String name;
	private int reqParams;
	private ExpressionActionDoer doer;
	
	private ExpressionAction(String name, int requiredParams, ExpressionActionDoer doer) {
		this.name = name;
		this.reqParams = requiredParams;
		this.doer = doer;
	}
	
	public String getName() {
		return name;
	}
	
	public int getRequiredParameters() {
		return reqParams;
	}
	
	public void doAction(Stack<String> stack, InputHandler inputHandler) {
		doer.doAction(stack, inputHandler);
	}
	
	private static interface ExpressionActionDoer {
		void doAction(Stack<String> stack, InputHandler inputHandler);
	}
}
