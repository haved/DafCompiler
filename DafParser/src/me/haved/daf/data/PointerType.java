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
	
	public void setMutable(boolean mutable) {
		this.mutable = mutable;
	}
	
	public Type getBase() {
		return base;
	}
	
	public void setBase(Type base) {
		this.base = base;
	}
}
