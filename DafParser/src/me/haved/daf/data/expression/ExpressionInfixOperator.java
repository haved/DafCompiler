package me.haved.daf.data.expression;

import me.haved.daf.lexer.tokens.TokenType;

public enum ExpressionInfixOperator {
	PLUS(TokenType.PLUS), MINUS(TokenType.MINUS), MULT(TokenType.MULT), DIVIDE(TokenType.DIVIDE), MODULO(TokenType.MODULO);
	
	private TokenType token;
	private boolean boolOperator;
	
	ExpressionInfixOperator(TokenType token, boolean boolOperator) {
		this.token = token;
		this.boolOperator = boolOperator;
	}
	
	private ExpressionInfixOperator(TokenType token) {
		this(token, false);
	}
	
	public boolean matchesToken(TokenType type) {
		return this.token == type;
	}
	
	public boolean evaluatesToBoolean() {
		return boolOperator;
	}
}
