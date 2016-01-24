package me.haved.dafParsed.node;

public abstract class Node implements NodeInterface {
	
	public abstract String getName();
	
	public abstract String compileSubnodesToString();
	
	public String compileToString() {
		return String.format("(%s, %s)", getName(), compileSubnodesToString());
	}
}
