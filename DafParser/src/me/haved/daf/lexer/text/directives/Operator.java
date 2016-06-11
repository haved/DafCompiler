package me.haved.daf.lexer.text.directives;

import static me.haved.daf.LogHelper.*;

import me.haved.daf.lexer.text.TextParserUtil;

public class Operator {
	
	private static final CanTakeType FALSE = n->false;
	private static final CanTakeType TRUE = n->true;
	
	public static final Operator[] OPERATORS = {
		new Operator("+",  Operator::doAddition, 								 2, TRUE,  TRUE),
		new Operator("-",  (objects) -> doIntegerOperation((a,b)->a-b, objects), 2, FALSE, TRUE),
		new Operator("*",  (objects) -> doIntegerOperation((a,b)->a*b, objects), 2, FALSE, TRUE),
		new Operator("/",  (objects) -> doIntegerOperation((a,b)->a/b, objects), 2, FALSE, TRUE),
		new Operator("%",  (objects) -> doIntegerOperation((a,b)->a%b, objects), 2, FALSE, TRUE),
		new Operator(">",  (objects) -> doIntegerOperation((a,b)->a>b?1:0, objects),  2, FALSE, TRUE),
		new Operator("<",  (objects) -> doIntegerOperation((a,b)->a<b?1:0, objects),  2, FALSE, TRUE),
		new Operator(">=", (objects) -> doIntegerOperation((a,b)->a>=b?1:0, objects), 2, FALSE, TRUE),
		new Operator("<=", (objects) -> doIntegerOperation((a,b)->a<=b?1:0, objects), 2, FALSE, TRUE),
		new Operator("!",   Operator::not, 1,  FALSE, TRUE),
		new Operator("==",  Operator::equals, 2,  TRUE,  TRUE), //We do care for integers in case '+' occurs 
		new Operator("!=",  Operator::notEquals, 2,  TRUE,  TRUE),
		new Operator("len", objects->Integer.toString(objects[0].toString().length()), 1, TRUE,  FALSE), //We only want the string
		new Operator("?", Operator::questionColon, 3, n -> n>0, n -> n==0), //The first argument must be int, the rest string
		new Operator("toChar", Operator::toChar, 1, FALSE, TRUE)//,
		//new Operator("toInt", Operator::toInt, 1, TRUE, FALSE),
		//new Operator("substring", Operator::substring, 3, TRUE, TRUE)
	};
	
	private String name;
	private int paramCount;
	private CanTakeType strings, ints;
	private OperatorDoer doer;
	
	public Operator(String name, OperatorDoer doer, int paramCount, CanTakeType strings, CanTakeType ints) {
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
	
	public boolean canTakeString(int argPos) {
		return strings.canTake(argPos);
	}
	
	public boolean canTakeInt(int argPos) {
		return ints.canTake(argPos);
	}
	
	interface CanTakeType {
		boolean canTake(int argPos);
	}
	
	interface OperatorDoer {
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
		return IfPPController.evaluateIntToBoolean((Integer)objects[0]) ? IfPPController.FALSE_STRING : IfPPController.TRUE_STRING;
	}
	
	private static String equals(Object...objects) {
		logAssert(objects.length==2);
		return objects[0].toString().equals(objects[1].toString()) 
				? IfPPController.TRUE_STRING : IfPPController.FALSE_STRING;
	}
	
	private static String notEquals(Object...objects) {
		return not(equals(objects));
	}
	
	private static String questionColon(Object...objects) {
		logAssert(objects.length == 3 && objects[0] instanceof Integer);
		return IfPPController.evaluateIntToBoolean((Integer)objects[0]) ? objects[1].toString() : objects[2].toString();
	}
	
	private static String toChar(Object...objects) {
		logAssert(objects.length == 1 && objects[0] instanceof Integer);
		int i = (Integer)objects[0];
		if(i > Character.MAX_VALUE) {
			log(ERROR, "A PP expression wanted to turn the integer %d into a char! Way too high!", i);
			return makeOperatorWarning("E");
		}
		
		char c = (char) i;
		
		if(!TextParserUtil.isLegalChar(c)) {
			log(ERROR, "%d toChar made little sense, as the resulting char '%c' is not known to be legal.", i, c);
			return makeOperatorWarning("?");
		}
		
		return Character.toString(c);
	}
	
	
	private static final String OPERATOR_WARNING = "#OP_WARNING ";
	
	public static String makeOperatorWarning(String output) {
		return String.format("%s%s", OPERATOR_WARNING, output);
	}
	
	public static String parseWarning(String text) {
		if(text.startsWith(OPERATOR_WARNING))
			return text.substring(OPERATOR_WARNING.length());
		return null;
	}
}