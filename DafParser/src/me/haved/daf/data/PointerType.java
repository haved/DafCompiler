package me.haved.daf.data;

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
	
	public Type getBase() {
		return base;
	}
}
