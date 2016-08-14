package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

public class BooleanConstantExpression extends NodeBase implements Expression {

	private boolean value;
	
	public BooleanConstantExpression(Token token) {
		value = false;
		if(token.getType() == TokenType.TRUE)
			value = true;
		logAssert(value || token.getType() == TokenType.FALSE);
		setPosition(token, token);
	}
	
	@Override
	public String getSignature() {
		return value ? "true" : "false";
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		logAssert(false);
	}
}
