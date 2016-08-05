package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

public class FunctionExpression extends NodeBase implements Expression {
	private FunctionParameter[] params;
	private Type returnType;
	private Statement statement;
	
	public FunctionExpression(FunctionParameter[] params, Type returnType, Statement statement) {
		this.params = params;
		this.returnType = returnType;
		this.statement = statement;
		logAssert(statement != null || statement == null); //TODO: Not this
		if(statement != null) {
			this.endLine = statement.getEndLine();
			this.endCol  = statement.getEndCol();
		}
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
		if(statement != null) //Should never be null, but oh well
			builder.append(statement.getSignature());
		//Semi-colon added by def / let statement
		return builder.toString();
	}
	
	public FunctionExpression setStart(Token token) {
		this.line = token.getLine();
		this.col = token.getCol();
		return this;
	}
}
