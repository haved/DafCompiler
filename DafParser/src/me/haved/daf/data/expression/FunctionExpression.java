package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.FunctionType;
import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

public class FunctionExpression extends NodeBase implements Expression {
	private FunctionType type;
	private Statement statement;
	
	public FunctionExpression(FunctionParameter[] params, Type returnType, Statement statement) {
		this.type = new FunctionType(params, returnType);
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
		if(type.params != null)
			for(int i = 0; i < type.params.length; i++) {
				if(i!=0)
					builder.append(", ");
				builder.append(type.params[i].getSignature());
			}
		builder.append(")");
		if(type.returnType != null) {
			builder.append(":").append(type.returnType.getSignature());
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
	
	public FunctionType getType() {
		return type;
	}

	public void codegenCppAsFunction(PrintWriter cpp, PrintWriter h, String name) {
		String signature = type.getCppSignature(name);
		h.print(signature);
		h.println(";");
		cpp.print(signature);
		statement.codegenStatementCpp(cpp);
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		logAssert(false);
	}
}
