package me.haved.dafParser.node;

import java.io.PrintWriter;

import me.haved.dafParser.semantic.TokenPosition;

public interface NodeInterface {
	public String getName();
	
	public String compileSubnodesToString();
	
	public String compileToString();
	
	public void FillFromTokens(TokenPosition tokens);
	
	public void PrintToCppWriter(PrintWriter writer);
	
	public void PrintToHeaderWriter(PrintWriter writer);
}
