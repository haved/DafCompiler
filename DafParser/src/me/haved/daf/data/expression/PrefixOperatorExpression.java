package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.syxer.Operators.PrefixOperator;

public class PrefixOperatorExpression extends NodeBase implements Expression {
	private Expression exp;
	private PrefixOperator op;
	
	public PrefixOperatorExpression(PrefixOperator op, Expression exp) {
		this.op = op;
		this.exp = exp;
	}
	
	@Override
	public String getSignature() {
		return String.format(op.isSpecial() ? "%s %s" : "%s%s", op.getName(), exp);
	}
	
	public Expression getBaseExpression() {
		return exp;
	}
	
	public PrefixOperator getOperator() {
		return op;
	}
}
