package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;

import static me.haved.daf.LogHelper.*;

public class FunctionExpression extends NodeBase implements Expression {
	private FunctionParameter[] params;
	private Type returnType;
	private Statement statement;
	
	public FunctionExpression(FunctionParameter[] params, Type returnType, Statement statement) {
		this.params = params;
		this.returnType = returnType;
		this.statement = statement;
		logAssert(statement != null);
	}
	
	@Override
	public String getSignature() {
		StringBuilder builder = new StringBuilder();
		builder.append("(");
		if(params != null)
			for(int i = 0; i < params.length; i++) {
				if(i!=0)
					builder.append(", ");
				builder.append(params[i].getSignature());
			}
		builder.append(")");
		if(returnType != null) {
			builder.append(":").append(returnType.getSignature());
		}
		builder.append(" ");
		builder.append(statement.getSignature());
		//Semi-colon added by def / let statement
		return builder.toString();
	}
}
