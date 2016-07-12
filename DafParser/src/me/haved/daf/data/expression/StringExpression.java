package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.PointerType;
import me.haved.daf.data.PrimitiveType;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.Token;

public class StringExpression extends NodeBase implements Expression {
	public static final PointerType CHAR_ARRAY = new PointerType(PrimitiveType.UINT8, false);
	
	private String text;
	
	public StringExpression(String text) {
		this.text = text;
	}
	
	public String getText() {
		return text;
	}
	
	@Override
	public boolean isTypeSet() {
		return true;
	}

	@Override
	public boolean tryEvaluatingType() {
		return false;
	}

	@Override
	public Type getType() {
		return CHAR_ARRAY;
	}
	
	public StringExpression setPosition(Token token) {
		setPosition(token, token);
		return this;
	}
}
