package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;

public class VariableExpression extends NodeBase implements Expression {
	
	private String name;
	
	public VariableExpression(String name) {
		this.name = name;
	}
	
	public String getName() {
		return name;
	}
	
	public String toString() {
		return name;
	}
}
