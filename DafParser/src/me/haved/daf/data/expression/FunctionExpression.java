package me.haved.daf.data.expression;

import java.beans.Statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.data.type.Type;

public class FunctionExpression extends NodeBase implements Expression {
	private FunctionParameter[] params;
	private Type returnType;
	private Statement statement;
	
	public FunctionExpression(FunctionParameter[] params, Type returnType, Statement statement) {
		this.params = params;
		this.returnType = returnType;
		this.statement = statement;
	}
	
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("(");
		if(params != null)
			for(int i = 0; i < params.length; i++) {
				if(i!=0)
					builder.append(", ");
				builder.append(params[i].getSignature());
			}
		builder.append(")");
		return builder.toString();
	}
}
