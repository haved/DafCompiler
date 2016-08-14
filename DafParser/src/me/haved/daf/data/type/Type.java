package me.haved.daf.data.type;

import java.io.PrintWriter;

public interface Type {
	
	public String getSignature();
	public boolean isInteger();
	public void codegenCpp(PrintWriter out);
}
