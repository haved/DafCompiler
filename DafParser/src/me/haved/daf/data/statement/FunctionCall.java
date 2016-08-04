package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.lexer.tokens.Token;

public class FunctionCall extends NodeBase implements Statement, Expression {
	private String name;
	private Expression[] parameters;
	
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
	public String getSignature() {
		StringBuilder out = new StringBuilder(name).append("(");
		if(parameters != null)
			for(int i = 0; i < parameters.length; i++) {
				if(i != 0)
					out.append(", ");
				out.append(parameters[i]);
			}
		
		return out.append(")").toString();
	}
	
	public FunctionCall setStart(Token token) {
		this.line = token.getLine();
		this.col = token.getCol();
		return this;
	}
	
	public FunctionCall setEnd(Token token) {
		this.endLine = token.getLine();
		this.endCol = token.getEndCol();
		return this;
	}
}