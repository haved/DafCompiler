package me.haved.dafParser.node;

import java.io.PrintWriter;
import java.util.ArrayList;

import me.haved.dafParser.lexical.TokenType;

import static me.haved.dafParser.LogHelper.*;

public class RootNode extends Node {

	private ArrayList<Definition> definitions;
	
	public RootNode() {
		definitions = new ArrayList<>();
	}
	
	@Override
	public void PrintToCppWriter(PrintWriter writer) {
		for(Definition definition:definitions) {
			definition.PrintToCppWriter(writer);
			writer.println();
		}
	}
	
	@Override
	public void PrintToHeaderWriter(PrintWriter writer) {
		for(Definition definition:definitions) {
			definition.PrintToHeaderWriter(writer);
			writer.println();
		}
	}
	
	@Override
	public String getName() {
		return "Root Node";
	}

	@Override
	public String compileSubnodesToString() {
		StringBuilder builder = new StringBuilder("(RootNode, ");
		for(int i = 0; i < definitions.size(); i++) {
			if(i>0)
				builder.append(", ");
			builder.append(definitions.get(i).compileToString());
		}
		builder.append(" )");
		return builder.toString();
	}
	
	public ArrayList<Definition> getDefinitions() {
		return definitions;
	}
}
