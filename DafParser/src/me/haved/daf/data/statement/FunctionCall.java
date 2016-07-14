package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;

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
	
	public String toString() {
		StringBuilder out = new StringBuilder(name).append("(");
		if(parameters != null)
			for(int i = 0; i < parameters.length; i++) {
				if(i != 0)
					out.append(" ,");
				out.append(parameters[i]);
			}
		
		return out.append(")").toString();
	}
}
