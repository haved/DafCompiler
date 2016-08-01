package me.haved.daf.data.expression;

import me.haved.daf.data.NodeBase;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

public class NumberConstantExpression extends NodeBase implements Expression {
	
	private boolean real;
	private boolean single;
	private boolean loong;
	private String text;
	
	public NumberConstantExpression(Token number) {
		this.text = number.getText();
		real = text.indexOf(TextParserUtil.REAL_DECIMAL_CHAR)>0;
		single = TextParserUtil.isFloatLetter(text.charAt(text.length()-1));
		loong = TextParserUtil.isLongLetter(text.charAt(text.length()-1));
		logAssert(single ? real : true);
		logAssert(loong ? !real : true);
		setPosition(number, number);
	}
	
	public String getText() {
		return text;
	}
	
	public boolean isReal() {
		return real;
	}
	
	public boolean isSinglePressicion() {
		return single;
	}
	
	public boolean isLong() {
		return loong;
	}
	
	@Override
	public String getSignature() {
		return text;
	}
}
