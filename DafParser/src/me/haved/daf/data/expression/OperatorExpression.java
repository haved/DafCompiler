package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.Primitive;
import me.haved.daf.data.PrimitiveType;
import me.haved.daf.data.Type;

public class OperatorExpression extends NodeBase implements Expression {
	private static final PrimitiveType IMMUTABLE_BOOLEAN = new PrimitiveType(Primitive.BOOLEAN, false);
	
	private Exception a, b;
	private ExpressionInfixOperator operator;
	
	private boolean typeEvaluated;
	private Type type;
	
	public OperatorExpression(Exception a, Exception b, ExpressionInfixOperator operator) {
		this.a = a;
		this.b = b;
		this.operator = operator;
		if(operator.evaluatesToBoolean()) {
			this.typeEvaluated = true;
			this.type = IMMUTABLE_BOOLEAN;
		}
	}
	
	
	@Override
	public boolean isTypeSet() {
		return typeEvaluated;
	}

	@Override
	public Type getType() {
		return type;
	}
}
