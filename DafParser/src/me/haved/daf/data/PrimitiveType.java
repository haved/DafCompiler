package me.haved.daf.data;

import me.haved.daf.lexer.tokens.TokenType;

public class PrimitiveType extends NodeBase implements Type {
	
	public static PrimitiveType BOOLEAN = new PrimitiveType(Primitive.BOOLEAN, false);
	public static PrimitiveType UINT8 = new PrimitiveType(Primitive.UINT8, false);
	
	public static PrimitiveType INT32 = new PrimitiveType(Primitive.INT32, false);
	public static PrimitiveType FLOAT = new PrimitiveType(Primitive.FLOAT, false);
	public static PrimitiveType DOUBLE = new PrimitiveType(Primitive.DOUBLE, false);
	
	private Primitive primitive;
	private boolean mutable;
	
	public PrimitiveType(Primitive primitive, boolean mutable) {
		this.primitive = primitive;
		this.mutable = mutable;
	}
	
	public boolean isMutable() {
		return mutable;
	}

	@Override
	public String getSignature() {
		return mutable ? String.format("%s %s", TokenType.MUT, primitive.getName()) : primitive.getName();
	}

	@Override
	public boolean isInteger() {
		return primitive.isInteger();
	}
}
