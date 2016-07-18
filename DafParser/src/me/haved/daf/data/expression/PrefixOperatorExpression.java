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
	public String toString() {
		return String.format(op.isSpecial() ? "%s %s" : "%s%s", op.getName(), exp);
	}
}
