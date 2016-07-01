package me.haved.daf.data;

import me.haved.daf.lexer.tokens.TokenType;

public class PointerType extends NodeBase implements Type {
	
	private Type base;
	private boolean mutable;
	
	public PointerType(Type base, boolean mutable) {
		this.base = base;
		this.mutable = mutable;
	}
	
	public boolean isMutable() {
		return mutable;
	}
	
	public void setMutable(boolean mutable) {
		this.mutable = mutable;
	}
	
	public Type getBase() {
		return base;
	}
	
	public void setBase(Type base) {
		this.base = base;
	}

	@Override
	public String getSignature() {
		if(mutable)
			return String.format("%s%s %s", TokenType.ADDRESS, TokenType.MUT, base.getSignature());
		else
			return String.format("%s%s", TokenType.ADDRESS, base.getSignature());
	}
}
