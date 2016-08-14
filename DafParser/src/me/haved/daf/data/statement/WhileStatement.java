package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

public class WhileStatement extends NodeBase implements Statement {

	private Expression condition;
	private Statement action;
	
	public WhileStatement(Expression condition, Statement action) {
		logAssert(condition != null);
		this.condition = condition;
		this.action = action;
	}
	
	@Override
	public String getSignature() {
		return String.format("while(%s) %s", condition.getSignature(), action);
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
	
	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		logAssert(false);
	}
}
