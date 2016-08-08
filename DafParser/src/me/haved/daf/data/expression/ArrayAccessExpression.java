package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;

public class ArrayAccessExpression extends NodeBase implements Expression {

	private Expression array, index;
	
	public ArrayAccessExpression(Expression array, Expression index) {
		this.array = array;
		this.index = index;
	}
	
	@Override
	public String getSignature() {
		return String.format("%s[%s]", array.getSignature(), index.getSignature());
	}
}