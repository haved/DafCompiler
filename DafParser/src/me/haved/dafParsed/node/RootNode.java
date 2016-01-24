package me.haved.dafParsed.node;

import java.util.ArrayList;

public class RootNode extends Node {

	private ArrayList<Definition> definitions;
	
	public RootNode() {
		definitions = new ArrayList<>();
	}
	
	@Override
	public String getName() {
		return "Root Node";
	}

	@Override
	public String compileSubnodesToString() {
		StringBuilder builder = new StringBuilder();
		for(int i = 0; i < definitions.size(); i++) {
			if(i>0)
				builder.append(", ");
			builder.append(definitions.get(i).compileToString());
		}
		return builder.toString();
	}
}
