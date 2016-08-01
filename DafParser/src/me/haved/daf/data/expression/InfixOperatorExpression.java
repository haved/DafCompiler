package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.syxer.Operators.InfixOperator;

public class InfixOperatorExpression extends NodeBase implements Expression {
	
	private Expression LHS, RHS;
	private InfixOperator op;
	
	public InfixOperatorExpression(Expression LHS, InfixOperator op, Expression RHS) {
		this.LHS = LHS;
		this.RHS = RHS;
		this.op = op;
	}
	
	@Override
	public String getSignature() {
		return String.format("(%s%s%s)", LHS, op.getText(), RHS);
	}
}
