package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.type.Type;

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
	
	@Override
	public boolean isTypeSet() {
		return false;
	}

	@Override
	public boolean tryEvaluatingType() {
		return false;
	}

	@Override
	public Type getType() {
		return null;
	}
}
