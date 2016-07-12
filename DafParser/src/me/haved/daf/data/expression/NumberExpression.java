package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.PrimitiveType;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class NumberExpression extends NodeBase implements Expression {
	
	private String text;
	private Type type;
	
	public NumberExpression(Token token) {
		
		String text = token.getText();
		
		if(token.getType() == TokenType.CHAR_LITTERAL) {
			type = PrimitiveType.UINT8; //char
			if(text.length() < 1) {
				log(token, ERROR, "A char litteral must have a length of at least 1");
				this.text = Integer.toString('E');
				return;
			}
			char c = text.charAt(0);
			if(c == '\\') {
				if(text.length() != 2) {
					log(token, ERROR, "A char litteral with a backslash must be two letters long (\\u is not possible)");
					text = Integer.toString('E');
				}
				c = text.charAt(1);
				switch(c) {
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case 'r': c = '\r'; break;
				case 'b': c = '\b'; break;
				case 'f': c = '\f'; break;
				case '\'':c = '\''; break;
				case '\"':c = '\"'; break;
				case '\\': break; // c is already correct
				case 'u': log(token, ERROR, "\\u is not supported in daf. Just write it as a number litteral, please");
				default: log(token, ERROR, "\\%c can't be evaluated to anything!", c); break;
				}
			}
			else if(text.length()!=1)
				log(token, ERROR, "A char can only be one letter long, unless it's a special code");
			
			this.text = Integer.toString(c);
		}
		
		this.type = text.contains(".") ? text.contains("f") ? PrimitiveType.FLOAT : PrimitiveType.DOUBLE : PrimitiveType.INT32;
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
		logAssert(false);
		return false;
	}

	@Override
	public Type getType() {
		return type;
	}
	
	public NumberExpression setPosition(Token token) {
		setPosition(token, token);
		return this;
	}
}
