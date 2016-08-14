package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

public class FunctionCall extends NodeBase implements Statement, Expression {
	private Expression expression;
	private Expression[] parameters;
	
	public FunctionCall(Expression expression, Expression[] parameters) {
		logAssert(expression != null);
		this.expression = expression;
		this.parameters = parameters;
	}
	
	public Expression getExpression() {
		return expression;
	}
	
	public boolean hasParameters() {
		return parameters != null && parameters.length > 0;
	}
	
	public Expression[] getParameters() {
		return parameters;
	}
	
	@Override
	public String getSignature() {
		StringBuilder out = new StringBuilder(expression.getSignature()).append("(");
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

	@Override
	public boolean isValidStatement() {
		return true;
	}
	
	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		codegenExpressionCpp(cpp);
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		logAssert(false);
	}
}