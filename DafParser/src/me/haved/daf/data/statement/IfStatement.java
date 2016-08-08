package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;

import static me.haved.daf.LogHelper.*;

public class IfStatement extends NodeBase implements Statement {

	private Expression conditional;
	private Statement action, elseAction;
	
	public IfStatement(Expression conditional, Statement action) {
		this(conditional, action, null);
	}
	
	public IfStatement(Expression conditional, Statement action, Statement elseAction) {
		logAssert(conditional != null && action != null);
		this.conditional = conditional;
		this.action = action;
		this.elseAction = elseAction;
	}
	
	@Override
	public String getSignature() {
		return elseAction == null ? String.format("if(%s)\n%s", conditional.getSignature(), action.getSignature()) :
			String.format("if(%s)\n%s\nelse\n%s", conditional.getSignature(), action.getSignature(), elseAction.getSignature());
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
}
