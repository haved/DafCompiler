package me.haved.dafParser.node;

import java.io.PrintWriter;
import java.util.ArrayList;

import me.haved.dafParser.lexical.Token;

public interface NodeInterface {
	public String getName();
	
	public String compileSubnodesToString();
	
	public String compileToString();
	
	public int FillFromTokens(ArrayList<Token> tokens, int start);
	
	public void PrintToCppWriter(PrintWriter writer);
	
	public void PrintToHeaderWriter(PrintWriter writer);
}
