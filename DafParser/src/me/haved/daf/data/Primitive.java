package me.haved.daf.data;

import me.haved.daf.lexer.tokens.TokenType;

public enum Primitive {
	
	UINT8(TokenType.UINT8, TokenType.UBYTE), 	INT8(TokenType.INT8, TokenType.CHAR),
	UINT16(TokenType.UINT16, TokenType.USHORT),	INT16(TokenType.INT16, TokenType.SHORT);
	
	private String name;
	private TokenType[] tokens;
	
	private Primitive(TokenType...tokens) {
		this.name = tokens[0].getText();
		this.tokens = tokens;
	}
}
