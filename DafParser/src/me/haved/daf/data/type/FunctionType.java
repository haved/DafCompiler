package me.haved.daf.data.type;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class FunctionType extends NodeBase implements Type {	
	public final FunctionParameter[] params;
	public final Type returnType;
	
	public FunctionType(FunctionParameter[] array, Type returnType) {
		this.params = array;
		this.returnType = returnType;
	}

	@Override
	public String getSignature() {
		StringBuilder sign = new StringBuilder();
		sign.append(TokenType.LEFT_PAREN);
		for(int i = 0; i < params.length; i++) {
			if(i!=0)
				sign.append(", ");
			sign.append(params[i].getSignature());
		}
		sign.append(TokenType.RIGHT_PAREN);
		if(returnType != null)
			sign.append(returnType.getSignature());
		return sign.toString();
	}

	@Override
	public boolean isInteger() {
		return false;
	}
	
	@Override
	public String codegenCpp() {
		logAssert(false);
		return null;
	}

	public String getCppSignature(String name) {
		StringBuilder out = new StringBuilder();
		if(returnType != null)
			out.append(returnType.codegenCpp()).append(" ");
		else
			out.append("void ");
		out.append(name).append("(");
		if(params != null)
			for(int i = 0; i < params.length; i++) {
				if(i != 0)
					out.append(", ");
				out.append(params[i].getType().codegenCpp());
				if(params[i].getName()!=null)
					out.append(" ").append(params[i].getName());
			}
		out.append(")");
		return out.toString();
	}
}
