package me.haved.daf.data.type;

import me.haved.daf.data.NodeBase;
import me.haved.daf.lexer.tokens.TokenType;

public class FunctionType extends NodeBase implements Type {
	public static class FunctionParameter {
		public static final int NOT_REFRENCE = 0;
		public static final int IMMUTABLE_REF = 1;
		public static final int MUTABLE_REF = 2;
		public static final int MOVE_REF = 3;
		
		private int refType;
		private String name;
		private Type type;
		
		public FunctionParameter(int refType, String name, Type type) {
			this.refType = refType;
			this.name = name;
			this.type = type;
		}
		
		private static final String[] REF_SIGNS = {"","&","&mut ","&move "};
		public String getSignature() {
			if(refType == NOT_REFRENCE && name==null)
				return type.getSignature();
			return String.format("%s%s:%s", REF_SIGNS[refType], name!=null?name:"", type.getSignature());
		}
		
		public int getReferenceType() {
			return refType;
		}
		
		public String getName() {
			return name;
		}
		
		public Type getType() {
			return type;
		}
	}
	
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
