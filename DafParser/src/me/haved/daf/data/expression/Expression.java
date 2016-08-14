package me.haved.daf.data.expression;

import java.io.PrintWriter;

import me.haved.daf.data.Node;
//import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;

public interface Expression extends Node {
//	public boolean isTypeSet();
//	public boolean tryEvaluatingType();
//	public Type getType();
	public void setPosition(Token start, Token end);
	public void codegenExpressionCpp(PrintWriter writer);
	
	public static void packExpressionInParenthesies(PrintWriter out, Expression expression) {
		out.print("(");
		expression.codegenExpressionCpp(out);
		out.print(")");
	}
}
