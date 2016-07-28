package me.haved.daf.data.type;

import me.haved.daf.lexer.tokens.TokenType;

public enum Primitive implements Type {
	UINT8	(TokenType.UINT8 , 	TokenType.UBYTE  , 			 TokenType.CHAR)	, INT8				(TokenType.INT8),
	UINT16	(TokenType.UINT16, 	TokenType.USHORT),	INT16	(TokenType.INT16	, TokenType.SHORT	),
	UINT32	(TokenType.UINT32, 	TokenType.UINT	),	INT32	(TokenType.INT32	, TokenType.INT		),
	UINT64	(TokenType.UINT64, 	TokenType.ULONG	),	INT64	(TokenType.INT64	, TokenType.LONG	),
	USIZE	(TokenType.USIZE					), 	BOOLEAN	(TokenType.BOOLEAN ), 
	FLOAT	(TokenType.FLOAT					), 	DOUBLE	(TokenType.DOUBLE  );
	
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
	
	public boolean isInteger() {
		return this != BOOLEAN && this != FLOAT && this != DOUBLE;
	}	

	@Override
	public String getSignature() {
		return name;
	}
	
	public static Primitive getPrimitiveForType(TokenType type) {
		for(Primitive p:Primitive.values())
			if(p.fitsTokenType(type))
				return p;
		return null;
	}
}
