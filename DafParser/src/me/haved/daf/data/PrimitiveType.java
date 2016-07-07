package me.haved.daf.data;

import me.haved.daf.lexer.tokens.TokenType;

public class PrimitiveType extends NodeBase implements Type {
	
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
}
