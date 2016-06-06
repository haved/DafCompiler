package me.haved.daf.lexer.text.directives;

import static me.haved.daf.LogHelper.*;

public class Operator {
	
	public static final Operator[] operators = {new Operator("+", Operator::doAddition, 2, true, true)};
	
	private String name;
	private int paramCount;
	private boolean strings, ints;
	private OperatorDoer doer;
	
	public Operator(String name, OperatorDoer doer, int paramCount, boolean strings, boolean ints) {
		this.name = name;
		this.doer = doer;
		this.paramCount = paramCount;
		this.strings = strings;
		this.ints = ints;
	}
	
	public String getName() {
		return name;
	}
	
	public OperatorDoer getDoer() {
		return doer;
	}
	
	public int getParamCount() {
		return paramCount;
	}
	
	public boolean canTakeStrings() {
		return strings;
	}
	
	public boolean canTakeInts() {
		return ints;
	}
	
	public interface OperatorDoer {
		String doOperator(Object... objects);
	}


	private static String doAddition(Object...objects) {
		logAssert(objects.length == 2);
		if(objects[0] instanceof Integer && objects[1] instanceof Integer) {
			return Integer.toString(((Integer)objects[0])+((Integer)objects[1]));
		}
		else {
			return objects[0].toString() + objects[1].toString();
		}
	}
}