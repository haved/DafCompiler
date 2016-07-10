package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.Primitive;
import me.haved.daf.data.PrimitiveType;
import me.haved.daf.data.Type;

import static me.haved.daf.LogHelper.*;

public class OperatorExpression extends NodeBase implements Expression {
	private static final PrimitiveType IMMUTABLE_BOOLEAN = new PrimitiveType(Primitive.BOOLEAN, false);
	
	private Expression a, b;
	private ExpressionInfixOperator operator;
	
	private boolean typeEvaluated;
	private Type type;
	
	public OperatorExpression(Expression a, Expression b, ExpressionInfixOperator operator) {
		this.a = a;
		this.b = b;
		this.operator = operator;
		if(this.operator.evaluatesToBoolean()) {
			this.typeEvaluated = true;
			this.type = IMMUTABLE_BOOLEAN;
		}
	}
	
	
	@Override
	public boolean isTypeSet() {
		return typeEvaluated;
	}

	@Override
	public boolean tryEvaluatingType() {
		logAssert(!typeEvaluated);
		
		if(!a.isTypeSet() && !a.tryEvaluatingType())
			return false;
		
		if(!b.isTypeSet() && !b.tryEvaluatingType())
			return false;
		
		
		
		return true;
	}
	
	@Override
	public Type getType() {
		return type;
	}
}
