package me.haved.daf.syxer;

import me.haved.daf.lexer.tokens.TokenType;

public class Operators {
	public static final int TAKES_NUMBERS_RETURNS_NUMBER = 0b0, TNRN = 0; // + - * / %
	public static final int TAKES_NUMBERS_RETURNS_BOOL = 0b1, TNRB = 0b1; // > >= < <=
	public static final int TAKES_VALUES_RETURNS_BOOL = 0b10, TVRB = 0b10; // == !=
	public static final int TAKES_INTEGERS_RETURNS_INT = 0b100, TIRI = 0b100; // &   | << >> >>>  //Bitwise
	public static final int TAKES_BOOLEANS_RETURNS_BOOL = 0b1000, TBRB = 0b1000; // &&  ||  //Logical
	public static final int TAKES_CLASS_AND_FIELD_RETURNS_VALUE = 0b10000, TCAFRV = 0b10000; // . ->
	
	public static enum InfixOperator {
		
		CLASS_ACCESS(TokenType.CLASS_ACCESS, 100, TCAFRV), POINTER_ACCESS(TokenType.POINTER_ACCESS, 100, TCAFRV),
		
		MULT(TokenType.MULT, 40, TNRN), DIVIDE(TokenType.DIVIDE, 40, TNRN),
		MODULO(TokenType.MODULO, 40, TNRN),
		
		PLUS(TokenType.PLUS, 30, TNRN), MINUS(TokenType.MINUS, 30, TNRN),
		
		GREATER(TokenType.GREATER, 20, TNRB), GREATER_OR_EQUAL(TokenType.GREATER_OR_EQUAL, 20, TNRB), 
		LOWER(TokenType.LOWER, 20, TNRB), LOWER_OR_EQUAL(TokenType.LOWER_OR_EQUAL, 20, TNRB),
		
		EQUALS(TokenType.EQUALS, 15, TVRB), NOT_EQUAL(TokenType.NOT_EQUALS, 15, TVRB),
		
		BITWISE_AND(TokenType.BITWISE_AND, 12, TIRI), BITWISE_XOR(TokenType.XOR, 10, TIRI), BITWISE_OR(TokenType.BITWISE_OR, 8, TIRI), 
		
		LOGICAL_AND(TokenType.LOGICAL_AND, 6, TBRB), LOGICAL_OR(TokenType.LOGICAL_OR, 4, TBRB);
		
		private TokenType tokenType;
		/** Higher level means higher priority. For instance: * has a higher level than +, because a+b*c==a+(b*c) */
		private int level;
		private int inOut;
		
		private InfixOperator(TokenType type, int level, int inOut) {
			this.tokenType = type;
			this.level = level;
			this.inOut = inOut;
		}
		
		/**
		 * The higher the level, the higher the evaluation priority of the operator.
		 * For example, multiplication has a higher level / priority than addition.
		 * This means a+b*b will be parsed as a+(b*c)
		 * If two operators have the same level (or the second had a lower), 
		 * the parsing will be straight-forward left to right
		 * 
		 * @return the level of the operator
		 */
		public int getLevel() {
			return level;
		}
		
		public boolean takesInteger() {
			return (inOut & (TNRN | TNRB | TVRB | TIRI)) != 0;
		}
		
		public boolean takesRealNumbers() {
			return (inOut & (TNRN | TNRB | TVRB)) != 0; //Note to self... give & a higher priority than == and !=
		}
		
		public boolean takesBoolean() {
			return (inOut & (TVRB | TBRB)) != 0;
		}
		
		public boolean returnsNumber() {
			return inOut == TNRN;
		}
		
		public boolean returnsInteger() {
			return inOut == TIRI;
		}
		
		public boolean returnsBoolean() {
			return (inOut & (TNRB | TVRB | TBRB)) != 0; 
		}
		
		public String getText() {
			return tokenType.getText();
		}
	}
	
	public static InfixOperator findInfixOperator(TokenType type) {
		for(InfixOperator op : InfixOperator.values()) {
			if(op.tokenType == type)
				return op;
		}
		return null;
	}
}