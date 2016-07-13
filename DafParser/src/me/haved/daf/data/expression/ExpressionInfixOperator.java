package me.haved.daf.data.expression;

import me.haved.daf.lexer.tokens.TokenType;

public enum ExpressionInfixOperator {
	PLUS(TokenType.PLUS), MINUS(TokenType.MINUS), MULT(TokenType.MULT), DIVIDE(TokenType.DIVIDE), MODULO(TokenType.MODULO),
	SHIFT_LEFT(TokenType.SHIFT_LEFT, false, true, true), LOGICAL_SHIFT_RIGHT(TokenType.LOGICAL_SHIFT_RIGHT, false, true, true), 
	ARITHMETIC_SHIFT_RIGHT(TokenType.ARITHMETIC_SHIFT_RIGHT, false, true, true),
	BITWISE_AND(TokenType.BITWISE_AND, false, false, true), LOGICAL_AND(TokenType.LOGICAL_AND, false, false, true), 
	BITWISE_OR(TokenType.BITWISE_OR, false, false, true), LOGICAL_OR(TokenType.LOGICAL_OR, false, false, true),
	BITWISE_XOR(TokenType.XOR, false, false, true),
	EQUALS(TokenType.EQUALS, true), NOT_EQUALS(TokenType.NOT_EQUALS, true), GREATER(TokenType.GREATER, true), LOWER(TokenType.LOWER, true), 
	GREATER_OR_EQUALS(TokenType.GREATER_OR_EQUAL, true), LOWER_OR_EQUALS(TokenType.LOWER_OR_EQUAL, true);
	
	private TokenType token;
	private boolean boolOperator;
	private boolean onlyMindFirstParam;
	private boolean integersOnly;
	
	ExpressionInfixOperator(TokenType token, boolean boolOperator, boolean onlyMindFirstParam, boolean integersOnly) {
		this.token = token;
		this.boolOperator = boolOperator;
		this.onlyMindFirstParam = onlyMindFirstParam;
		this.integersOnly = integersOnly;
	}
	
	ExpressionInfixOperator(TokenType token, boolean boolOperator) {
		this(token, boolOperator, false, false);
	}
	
	ExpressionInfixOperator(TokenType token) {
		this(token, false, false, false);
	}
	
	public boolean matchesToken(TokenType type) {
		return this.token == type;
	}
	
	public boolean evaluatesToBoolean() {
		return boolOperator;
	}
	
	public boolean onlyMindFirstOperand() {
		return onlyMindFirstParam;
	}
	
	public boolean integersOnly() {
		return integersOnly;
	}
	
	public String getText() {
		return token.getText();
	}
}
