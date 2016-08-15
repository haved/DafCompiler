package me.haved.daf.data.expression;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.syxer.Operators.InfixOperator;

import static me.haved.daf.LogHelper.*
;
public class InfixOperatorExpression extends NodeBase implements Expression, Statement {
	
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

	@Override
	public boolean isValidStatement() {
		return op.isStatement();
	}

	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		logAssert(isValidStatement());
		codegenExpressionCpp(cpp);
		cpp.println(";");
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		LHS.codegenExpressionCpp(cpp);
		cpp.print(op.getCppName());
		RHS.codegenExpressionCpp(cpp);
	}
}
