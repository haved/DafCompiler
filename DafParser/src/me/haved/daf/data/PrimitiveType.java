package me.haved.daf.data;

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
		return primitive.getName();
	}
}
