package me.haved.daf.data;

import me.haved.daf.lexer.tokens.TokenType;

public enum Primitive {
	
	UINT8(TokenType.UINT8, TokenType.UBYTE), 	INT8(TokenType.INT8, TokenType.CHAR),
	UINT16(TokenType.UINT16, TokenType.USHORT),	INT16(TokenType.INT16, TokenType.SHORT),
	UINT32(TokenType.UINT32, TokenType.UINT),	INT32(TokenType.INT32, TokenType.INT),
	UINT64(TokenType.UINT64, TokenType.ULONG),	INT64(TokenType.INT64, TokenType.LONG);
	
	private String name;
	private TokenType[] tokens;
	
	private Primitive(TokenType...tokens) {
		this.name = tokens[0].getText();
		this.tokens = tokens;
	}
	
	public String getName() {
		return name;
	}
	
	public boolean fitsTokenType(TokenType type) {
		for(int i = 0; i < tokens.length; i++)
			if(tokens[i]==type)
				return true;
		return false;
	}
}
