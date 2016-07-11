package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.PrimitiveType;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class NumberExpression extends NodeBase implements Expression {
	
	private long value;
	private Type type;
	
	public NumberExpression(Token token) {
		
		if(token.getType() == TokenType.CHAR_LITTERAL) {
			type = PrimitiveType.UINT8; //char
			if(token.getText().length() < 1) {
				log(token, ERROR, "A char litteral must have a length of at least 1");
				value = 'E';
				return;
			}
			char c = token.getText().charAt(0);
			if(c == '\\') {
				if(token.getText().length() < 2) {
					log(token, ERROR, "A char litteral can't have a lone backslash");
					value = 'E';
				}
			}
		}
		else {
			this.text = token.getText();
			if(text.contains(".")) {
				type = text.endsWith("f") ? PrimitiveType.FLOAT : PrimitiveType.DOUBLE;
			}
			else {
				if(text.contains("f")) {
					log(token, ERROR, "A number litteral may not be a float without a decimal point");
					text.substring(0, text.indexOf('f'));
				}
				type = PrimitiveType.INT32;
			}
		}
	}
	
	@Override
	public boolean isTypeSet() {
		return true;
	}

	@Override
	public boolean tryEvaluatingType() {
		logAssert(false);
		return false;
	}

	@Override
	public Type getType() {
		return type;
	}
}
