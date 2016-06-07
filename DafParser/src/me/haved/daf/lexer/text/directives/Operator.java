package me.haved.daf.lexer.text.directives;

import static me.haved.daf.LogHelper.*;

public class Operator {
	
	public static final Operator[] operators = {
		new Operator("+",  Operator::doAddition, 								 2, true,  true),
		new Operator("-",  (objects) -> doIntegerOperation((a,b)->a-b, objects), 2, false, true),
		new Operator("*",  (objects) -> doIntegerOperation((a,b)->a*b, objects), 2, false, true),
		new Operator("/",  (objects) -> doIntegerOperation((a,b)->a/b, objects), 2, false, true),
		new Operator("%",  (objects) -> doIntegerOperation((a,b)->a%b, objects), 2, false, true),
		new Operator(">",  (objects) -> doIntegerOperation((a,b)->a>b?1:0, objects),  2, false, true),
		new Operator("<",  (objects) -> doIntegerOperation((a,b)->a<b?1:0, objects),  2, false, true),
		new Operator(">=", (objects) -> doIntegerOperation((a,b)->a>=b?1:0, objects), 2, false, true),
		new Operator("<=", (objects) -> doIntegerOperation((a,b)->a<=b?1:0, objects), 2, false, true),
		new Operator("!",   Operator::not, 1,  false, true),
		new Operator("==",  Operator::equals, 2,  true,  true),
		new Operator("!=",  Operator::notEquals, 2,  true,  true),
		new Operator("len", objects->Integer.toString(objects[0].toString().length()), 1, true,  true)
	};
	
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
	
	interface IntegerOperatorDoer {
		int doIntegerOpperator(int a, int b);
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
	
	private static String doIntegerOperation(IntegerOperatorDoer doer, Object...objects) {
		logAssert(objects.length == 2);
		logAssert(objects[0] instanceof Integer && objects[1] instanceof Integer);
		return Integer.toString(doer.doIntegerOpperator((Integer)objects[0], (Integer)objects[1]));
	}
	
	private static String not(Object...objects) {
		logAssert(objects.length==1);
		logAssert(objects[0] instanceof Integer);
		return ((Integer)objects[0]) != 0 ? "1" : "0";
	}
	
	private static String equals(Object...objects) {
		logAssert(objects.length==2);
		return objects[0].toString().equals(objects[1].toString()) 
				? IfPPController.TRUE_STRING : IfPPController.FALSE_STRING;
	}
	
	private static String notEquals(Object...objects) {
		return not(equals(objects));
	}
}