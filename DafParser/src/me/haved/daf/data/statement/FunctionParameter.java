package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.type.Type;

public class FunctionParameter extends NodeBase {
	public static final int NOT_A_REF = 0;
	public static final int CONST_REF = 1;
	public static final int MUTBL_REF = 2;
	public static final int MOVES_REF = 3;
	private int refType;
	private String name;
	private Type type;
	
	public FunctionParameter(int refType, String name, Type type) {
		this.refType = refType;
		this.name = name;
		this.type = type;
	}
	
	public FunctionParameter(String name, Type type) {
		this.name = name;
		this.type = type;
		this.refType = NOT_A_REF;
	}
	
	private static final String[] REF_SIGNS = {"","&","&mut ","&move "};
	public String getSignature() {
		if(refType == NOT_A_REF && name==null)
			return type.getSignature();
		return String.format("%s%s:%s", REF_SIGNS[refType], name!=null?name:"", type.getSignature());
	}
	
	public boolean isReference() {
		return refType != NOT_A_REF;
	}
	
	public boolean mutableRefrence() {
		return refType == MUTBL_REF || refType == MOVES_REF;
	}
	
	public String getName() {
		return name;
	}
	
	public Type getType() {
		return type;
	}
}
