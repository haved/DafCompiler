package me.haved.daf.data.type;

import java.io.PrintWriter;

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

	@Override
	public void codegenCpp(PrintWriter out) {
		
	}
}
