package me.haved.daf.data;

public class FunctionParameter extends NodeBase {
	public static final int NOT_A_REF = 0;
	public static final int CONST_REF = 1;
	public static final int MUTBL_REF = 2;
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
	}
}
