package me.haved.daf.data.expression;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;
import me.haved.daf.lexer.tokens.Token;

public class VariableExpression extends NodeBase implements Expression {
	
	private String name;
	
	public VariableExpression(String name) {
		this.name = name;
	}
	
	public String getName() {
		return name;
	}
	
	@Override
	public String getSignature() {
		return name;
	}
	
	public VariableExpression setPosition(Token token) {
		super.setPosition(token, token);
		return this;
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		cpp.print(name);
	}
}
