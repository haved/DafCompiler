package me.haved.daf.data;

import me.haved.daf.data.expression.Expression;

public class FunctionCall extends NodeBase implements Statement, Expression {

	private String name;
	private Type returnType;
	private Expression[] parameters;
	private boolean returnTypeEvaluated;
	
	public FunctionCall(String name, Expression[] parameters) {
		this.name = name;
		this.parameters = parameters;
	}
	
	public String getName() {
		return name;
	}
	
	public boolean hasParameters() {
		return parameters != null && parameters.length > 0;
	}
	
	public Expression[] getParameters() {
		return parameters;
	}
	
	@Override
	public boolean isTypeSet() {
		return returnTypeEvaluated;
	}

	@Override
	public boolean tryEvaluatingType() {
		return false;
	}

	@Override
	public Type getType() {
		return returnType;
	}
}
