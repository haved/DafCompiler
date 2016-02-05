package me.haved.dafParser.node;

import java.io.PrintWriter;

import me.haved.dafParser.semantic.TokenPosition;

public abstract class Node implements NodeInterface {
	
	public abstract String getName();
	
	public abstract String compileSubnodesToString();
	
	public String compileToString() {
		return String.format("(%s, %s)", getName(), compileSubnodesToString());
	}
	
	public void FillFromTokens(TokenPosition tokens) {
		
	}
	
	public void PrintToCppWriter(PrintWriter writer) {
		
	}
	
	public void PrintToHeaderWriter(PrintWriter writer) {
		
	}
}
