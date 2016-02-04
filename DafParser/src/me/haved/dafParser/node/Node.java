package me.haved.dafParser.node;

import java.io.PrintWriter;
import java.util.ArrayList;

import me.haved.dafParser.lexical.Token;

public abstract class Node implements NodeInterface {
	
	public abstract String getName();
	
	public abstract String compileSubnodesToString();
	
	public String compileToString() {
		return String.format("(%s, %s)", getName(), compileSubnodesToString());
	}
	
	public int FillFromTokens(ArrayList<Token> tokens, int start) {
		return start;
	}
	
	public void PrintToCppWriter(PrintWriter writer) {
		
	}
	
	public void PrintToHeaderWriter(PrintWriter writer) {
		
	}
}
