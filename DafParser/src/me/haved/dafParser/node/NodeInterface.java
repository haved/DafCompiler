package me.haved.dafParser.node;

import java.io.PrintWriter;

public interface NodeInterface {
	public String getName();
	
	public String compileSubnodesToString();
	
	public String compileToString();
	
	public void PrintToCppWriter(PrintWriter writer);
	
	public void PrintToHeaderWriter(PrintWriter writer);
}
