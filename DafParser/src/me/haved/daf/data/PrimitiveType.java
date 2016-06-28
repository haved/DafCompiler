package me.haved.daf.data;

public class PrimitiveType extends NodeBase implements Type {
	
	private boolean mutable;
	
	public PrimitiveType(Primitive primitive, boolean mutable) {
		this.mutable = mutable;
	}
	
	public boolean isMutable() {
		return mutable;
	}
}
