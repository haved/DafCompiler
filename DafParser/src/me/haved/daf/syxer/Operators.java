package me.haved.daf.syxer;

import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import me.haved.daf.data.expression.Expression;

public class Operators {
	
	public static enum InfixOperator {
		
		MODULE_ACCESS(TokenType.MODULE_ACCESS, 1000),
		
		CLASS_ACCESS(TokenType.CLASS_ACCESS, 100), POINTER_ACCESS(TokenType.POINTER_ACCESS, 100),
		
		MULT(TokenType.MULT, 70), DIVIDE(TokenType.DIVIDE, 70),
		MODULO(TokenType.MODULO, 70),
		
		PLUS(TokenType.PLUS, 60), MINUS(TokenType.MINUS, 60),
		
		BITWISE_LEFT_SHIFT(TokenType.SHIFT_LEFT, 50), BITWISE_RIGHT_SHIFT(TokenType.ARITHMETIC_SHIFT_RIGHT, 50),
		
		GREATER(TokenType.GREATER, 40), GREATER_OR_EQUAL(TokenType.GREATER_OR_EQUAL, 40), 
		LOWER(TokenType.LOWER, 40), LOWER_OR_EQUAL(TokenType.LOWER_OR_EQUAL, 40),
		
		EQUALS(TokenType.EQUALS, 30), NOT_EQUAL(TokenType.NOT_EQUALS, 30),
		
		BITWISE_AND(TokenType.BITWISE_AND, 20), BITWISE_XOR(TokenType.XOR, 15), BITWISE_OR(TokenType.BITWISE_OR, 10), 
		
		LOGICAL_AND(TokenType.LOGICAL_AND, 8), LOGICAL_OR(TokenType.LOGICAL_OR, 6),
		
		ASSIGN(TokenType.ASSIGN, 4, true);
		
		private TokenType tokenType;
		private int precedence;
		private boolean statement;
		
		private InfixOperator(TokenType type, int precedence) {
			this(type, precedence, false);
		}
		
		private InfixOperator(TokenType type, int precedence, boolean statement) {
			this.tokenType = type;
			this.precedence = precedence;
			this.statement = statement;
		}
		
		public int getPrecedence() {
			return precedence;
		}
		
		public String getText() {
			return tokenType.getText();
		}
		
		public boolean isStatement() {
			return statement;
		}
	}
	
	public static InfixOperator findInfixOperator(TokenBufferer bufferer) {
		if(bufferer.hasCurrentToken()) {
			TokenType type = bufferer.getCurrentToken().getType();
			for(InfixOperator op : InfixOperator.values()) {
				if(op.tokenType == type)
					return op;
			}
		}
		return null;
	}
	
	public static enum PrefixOperator {
		MINUS(TokenType.MINUS, 90), PLUS(TokenType.PLUS, 90), ADDRESS(TokenType.getAddressType(), 90), 
		MUT_ADDRESS(TokenType.getAddressType().getText()+TokenType.MUT, 90), 
		SHARED_ADDRESS(TokenType.getAddressType().getText()+TokenType.SHARED, 90), 
		UNIQUE_ADDRESS(TokenType.getAddressType().getText()+TokenType.UNIQUE, 90), 
		DEREFERENCE(TokenType.DEREFERENCE, 90), NOT(TokenType.NOT, 90), BITWISE_NOT(TokenType.getBitwiseNot(), 90),
		PLUSS_PLUSS(TokenType.PLUS_PLUS, 90, true), MINUS_MINUS(TokenType.MINUS_MINUS, 90, true),
		SIZEOF(TokenType.SIZEOF, 90);
		
		private TokenType tokenType;
		private int precedence;
		private String name;
		private boolean statement;
		
		private PrefixOperator(TokenType tokenType, int precedence) {
			this(tokenType, precedence, false);
		}
		
		private PrefixOperator(TokenType tokenType, int precedence, boolean statement) {
			this.tokenType = tokenType;
			this.name = tokenType.getText();
			this.precedence = precedence;
			this.statement = statement;
		}
		
		private PrefixOperator(String name, int precedece) {
			this.name = name;
			this.precedence = precedece;
		}
		
		public boolean isSpecial() {
			return tokenType == null;
		}
		
		public TokenType getType() {
			return tokenType;
		}

		public String getName() {
			return name;
		}
		
		public int getPrecedence() {
			return precedence;
		}
		
		@Override
		public String toString() {
			return getName();
		}
		
		public boolean isStatement() {
			return statement;
		}
	}
	
	/**
	 * Eats the tokens that are part of a prefix operator
	 * Expects a current token
	 * 
	 * @param bufferer
	 * @return an operator if it's found, else null
	 */
	public static PrefixOperator parsePrefixOperator(TokenBufferer bufferer) {
		logAssert(bufferer.hasCurrentToken());
		PrefixOperator answer = null;
		TokenType type = bufferer.getCurrentToken().getType();
		if(type == TokenType.LOGICAL_AND)
			log(bufferer.getCurrentToken(), SUGGESTION, "If you really want a pointer to a pointer, stick a space inbetween");
		for(PrefixOperator op:PrefixOperator.values()) {
			if(!op.isSpecial() && op.getType() == type) {
				answer = op;
				break;
			}
		}
		if(answer == null)
			return null;
		
		bufferer.advance();
		if(answer == PrefixOperator.ADDRESS) {
			if(!bufferer.hasCurrentToken())
				return answer;
			type = bufferer.getCurrentToken().getType();
			if(type == TokenType.MUT)
				answer = PrefixOperator.MUT_ADDRESS;
			else if(type == TokenType.SHARED)
				answer = PrefixOperator.SHARED_ADDRESS;
			else if(type == TokenType.UNIQUE)
				answer = PrefixOperator.UNIQUE_ADDRESS;
			else
				return answer;
			bufferer.advance();
		}
		return answer;
	}

	public static enum PostfixOperator {
		PLUS_PLUS(TokenType.PLUS_PLUS, ExpressionParser::parsePostCrementExpression, 100), 
		MINUS_MINUS(TokenType.MINUS_MINUS, ExpressionParser::parsePostCrementExpression, 100), 
		FUNCTION_CALL(TokenType.LEFT_PAREN, ExpressionParser::parseFunctionCall, 100), 
		ARRAY_ACCESS(TokenType.LEFT_BRACKET, ExpressionParser::parseArrayAccess, 100);
		
		private TokenType type;
		private PostfixEvaluator eval;
		private int precedence;
		
		private PostfixOperator(TokenType firstToken, PostfixEvaluator eval, int precedence) {
			this.type = firstToken;
			this.eval = eval;
			this.precedence = precedence;
		}
		
		public TokenType getFirstToken() {
			return type;
		}
		
		public Expression evaluate(Expression LHS, TokenBufferer bufferer) {
			if(eval != null)
				return eval.evaluatePostfix(LHS, bufferer);
			return null;
		}
		
		public int getPrecedence() {
			return precedence;
		}
	}
	
	public static interface PostfixEvaluator {
		Expression evaluatePostfix(Expression LHS, TokenBufferer bufferer);
	}

	public static PostfixOperator findPostfixOperator(TokenBufferer bufferer) {
		if(bufferer.hasCurrentToken())
			for(PostfixOperator op:PostfixOperator.values()) {
				if(bufferer.isCurrentTokenOfType(op.getFirstToken())) {
					return op;
				}
			}
		return null;
	}
}