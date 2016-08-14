package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

public class StringConstantExpression extends NodeBase implements Expression {

	private String text;
	
	public StringConstantExpression(Token token) {
		logAssert(token != null && token.getType() == TokenType.STRING_LITERAL);
		this.text = token.getText();
		setPosition(token, token);
	}
	
	@Override
	public String getSignature() {
		return String.format("\"%s\"", text);
	}
	
	@Override
	public void codegenExpressionCpp(PrintWriter cpp) {
		cpp.printf("\"%s\"", text);
	}
}
