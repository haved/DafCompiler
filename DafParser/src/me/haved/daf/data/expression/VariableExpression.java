package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.Token;

public class VariableExpression extends NodeBase implements Expression {

	private String name;
	private Type type;
	private boolean typeEvaluated;
	
	public VariableExpression(String name) {
		this.name = name;
		this.typeEvaluated = false;
		this.type = null;
	}
	
	public String getName() {
		return name;
	}
	
	@Override
	public boolean isTypeSet() {
		return typeEvaluated;
	}

	@Override
	public boolean tryEvaluatingType() {
		return false;
	}

	@Override
	public Type getType() {
		return type;
	}
	
	public VariableExpression setVariablePosition(Token token) {
		this.setPosition(token, token);
		return this;
	}
}
