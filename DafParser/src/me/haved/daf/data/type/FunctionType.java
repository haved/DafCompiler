package me.haved.daf.data.type;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.lexer.tokens.TokenType;

public class FunctionType extends NodeBase implements Type {	
	private final FunctionParameter[] params;
	private final Type returnType;
	
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
}
