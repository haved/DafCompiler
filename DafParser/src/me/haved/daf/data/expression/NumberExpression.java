package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.PrimitiveType;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

public class NumberExpression extends NodeBase implements Expression {
	
	private String text;
	private Type type;
	
	public NumberExpression(Token token) {
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
