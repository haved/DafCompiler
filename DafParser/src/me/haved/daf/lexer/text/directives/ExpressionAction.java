package me.haved.daf.lexer.text.directives;

import java.util.Stack;

import me.haved.daf.lexer.text.PreProcessor.InputHandler;

public class ExpressionAction {
	//swap, dup, Xswap, lineNum, colNum, exists, macroStack 
	
	private static interface ExpressionActionDoer {
		void doAction(Stack<String> stack, InputHandler macroKeeper);
	}
}
