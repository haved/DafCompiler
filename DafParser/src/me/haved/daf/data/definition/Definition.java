package me.haved.daf.data.definition;

import java.io.PrintWriter;

import me.haved.daf.data.Node;

public interface Definition extends Node {
	public boolean isPublic();
	
	public void codegenDefinitionCpp(PrintWriter cpp, PrintWriter h);
}
