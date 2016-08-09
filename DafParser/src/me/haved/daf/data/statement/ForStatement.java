package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;

import static me.haved.daf.LogHelper.*;

public class ForStatement extends NodeBase implements Statement {
    
    private Statement initial;
    private Expression conditional;
    private Statement increment;
    private Statement action;

    public ForStatement(Statement initial, Expression conditional, Statement increment, Statement action) {
    	logAssert(conditional != null);
		this.initial = initial;
		this.conditional = conditional;
		this.increment = increment;
		this.action = action;
    }
    
    @Override
    public String getSignature() {
    	return String.format("for(%s %s; %s) %s", initial, conditional, increment, action);
    }

    @Override
    public boolean isValidStatement() {
    	return true;
    }
}
