package me.haved.daf.data.type;

public interface Type {
	
	public String getSignature();
	public boolean isInteger();
	public String codegenCpp();
}
