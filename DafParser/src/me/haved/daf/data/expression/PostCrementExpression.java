package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.Statement;

import static me.haved.daf.LogHelper.*;

public class PostCrementExpression extends NodeBase implements Expression, Statement {
	
	public static final int INCREMENT = 0;
	public static final int DECREMENT = 1;
	
	private Expression expression;
	private int crement;
	
	public PostCrementExpression(Expression expression, int crement) {
		this.expression = expression;
		this.crement = crement;
	}
	
	@Override
	public String getSignature() {
		if(crement == INCREMENT)
			return String.format("%s++", expression.getSignature());
		else if(crement == DECREMENT)
			return String.format("%s--", expression.getSignature());
		logAssert(false);
		return null;
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
}
