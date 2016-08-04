package me.haved.daf.data.type;

public class TypedefType implements Type {
	
	private String ident;
	
	public TypedefType(String ident) {
		this.ident = ident;
	}
	
	@Override
	public String getSignature() {
		return ident;
	}

	@Override
	public boolean isInteger() {
		return false;
	}
}