package me.haved.daf.data.type;

import me.haved.daf.data.NodeBase;
import me.haved.daf.lexer.tokens.TokenType;

public class PointerType extends NodeBase implements Type {
	private Type target;
	private boolean mutable;
	
	public PointerType(Type target, boolean mutable) {
		this.target = target;
		this.mutable = mutable;
	}
	
	/**
	 *  @return whether or not the type pointed to is mutable
	 */
	public boolean isMutable() {
		return mutable;
	}
	
	public void setMutable(boolean mutable) {
		this.mutable = mutable;
	}
	
	public Type getTarget() {
		return target;
	}
	
	public void setTarget(Type target) {
		this.target = target;
	}

	@Override
	public String getSignature() {
		if(mutable)
			return String.format("%s%s %s", TokenType.getAddressType(), TokenType.MUT, target.getSignature());
		else
			return String.format("%s%s", TokenType.getAddressType(), target.getSignature());
	}

	@Override
	public boolean isInteger() {
		return true; //A pointer is an integer in memory
	}
}
