package me.haved.daf.data.type;

import java.io.PrintWriter;

import me.haved.daf.lexer.tokens.TokenType;

public enum Primitive implements Type {
	CHAR(  TokenType.CHAR,   "char"),	  	UCHAR(	TokenType.UCHAR, "unsigned char"),
	SHORT( TokenType.SHORT,  "short"), 		USHORT(	TokenType.USHORT, "unsigned short"),
	INT(   TokenType.INT,	 "int"),	  	UINT(	TokenType.UINT,   "unsigned int"),
	LONG(  TokenType.LONG,   "long"),  		ULONG(	TokenType.ULONG,  "unsigned long"),
	UINT8( TokenType.UINT8,  "uint8_t"), 	INT8(  	TokenType.INT8, "int8_t"),
	UINT16(TokenType.UINT16, "uint16_t"),	INT16(	TokenType.INT16, "int16_t"),
	UINT32(TokenType.UINT32, "uint32_t"),	INT32(	TokenType.INT32, "int32_t"),
	UINT64(TokenType.UINT64, "uint64_t"),	INT64(	TokenType.INT64, "int64_t"),
  /*USIZE( TokenType.USIZE,  ""),*/ 		BOOLEAN(TokenType.BOOLEAN, "bool"), 
	FLOAT( TokenType.FLOAT, "float"), 		DOUBLE(	TokenType.DOUBLE, "double");
	
	private String name;
	private TokenType token;
	private String cppName;
	
	private Primitive(TokenType token, String cppName) {
		this.name = token.getText();
		this.token = token;
		this.cppName = cppName;
	}
	
	public String getName() {
		return name;
	}
	
	public boolean fitsTokenType(TokenType type) {
		return type == token;
	}
	
	public boolean isInteger() {
		return this != FLOAT && this != DOUBLE;
	}	

	@Override
	public String getSignature() {
		return name;
	}
	
	@Override
	public void codegenCpp(PrintWriter out) {
		out.print(cppName);
	}
	
	public static Primitive getPrimitiveForType(TokenType type) {
		for(Primitive p:Primitive.values())
			if(p.fitsTokenType(type))
				return p;
		return null;
	}
}
