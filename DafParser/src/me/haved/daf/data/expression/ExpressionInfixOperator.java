package me.haved.daf.data.expression;

import me.haved.daf.lexer.tokens.TokenType;

public enum ExpressionInfixOperator {
	PLUS(TokenType.PLUS), MINUS(TokenType.MINUS), MULT(TokenType.MULT), DIVIDE(TokenType.DIVIDE), MODULO(TokenType.MODULO),
	SHIFT_LEFT(TokenType.SHIFT_LEFT), LOGICAL_SHIFT_RIGHT(TokenType.LOGICAL_SHIFT_RIGHT), ARITHMETIC_SHIFT_RIGHT(TokenType.ARITHMETIC_SHIFT_RIGHT),
	BITWISE_AND(TokenType.BITWISE_AND), LOGICAL_AND(TokenType.LOGICAL_AND), BITWISE_OR(TokenType.BITWISE_OR), LOGICAL_OR(TokenType.LOGICAL_OR),
	BITWISE_XOR(TokenType.XOR);
	
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
