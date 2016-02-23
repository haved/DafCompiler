package me.haved.dafParser.node;

import java.io.PrintWriter;

public abstract class Node implements NodeInterface {
	
	public abstract String getName();
	
	public abstract String compileSubnodesToString();
	
	public String compileToString() {
		return String.format("(%s, %s)", getName(), compileSubnodesToString());
	}
	
	public void PrintToCppWriter(PrintWriter writer) {
		
	}
	
	public void PrintToHeaderWriter(PrintWriter writer) {
		
	}
}
