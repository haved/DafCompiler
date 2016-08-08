package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;

import static me.haved.daf.LogHelper.*;

public class WhileStatement extends NodeBase implements Statement, Expression{

	private Expression condition;
	private Statement action;
	
	public WhileStatement(Expression condition, Statement action) {
		logAssert(condition != null && action != null);
		this.condition = condition;
		this.action = action;
	}
	
	@Override
	public String getSignature() {
		return String.format("while(%s) %s", condition.getSignature(), action.getSignature());
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
}
