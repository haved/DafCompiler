package me.haved.daf.data.expression;

import static me.haved.daf.LogHelper.logAssert;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.syxer.Operators.PrefixOperator;

public class PrefixOperatorExpression extends NodeBase implements Expression, Statement {
	private Expression exp;
	private PrefixOperator op;
	
	public PrefixOperatorExpression(PrefixOperator op, Expression exp) {
		this.op = op;
		this.exp = exp;
		if(exp!=null) {
			this.endLine = exp.getEndLine();
			this.endCol = exp.getEndCol();
		}
	}
	
	@Override
	public String getSignature() {
		return String.format(op.isSpecial() ? "\"%s %s\"" : "\"%s%s\"", op.getName(), exp);
	}
	
	public Expression getBaseExpression() {
		return exp;
	}
	
	public PrefixOperator getOperator() {
		return op;
	}

	public PrefixOperatorExpression setStart(Token token) {
		this.line = token.getLine();
		this.col = token.getCol();
		return this;
	}

	@Override
	public boolean isValidStatement() {
		return op.isStatement();
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		cpp.print(op.getCppName());
		Expression.packExpressionInParenthesies(cpp, exp);
	}
	
	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		logAssert(isValidStatement());
		codegenExpressionCpp(cpp);
		cpp.println(";");
	}
}
