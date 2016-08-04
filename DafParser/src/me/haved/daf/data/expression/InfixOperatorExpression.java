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
		if(LHS!=null) {
			this.line = LHS.getLine();
			this.col = LHS.getCol();
		}
		if(RHS!=null) {
			this.endLine = RHS.getEndLine();
			this.endCol = RHS.getEndCol();
		}
	}
	
	@Override
	public String getSignature() {
		return String.format("(%s%s%s)", LHS, op.getText(), RHS);
	}
}
