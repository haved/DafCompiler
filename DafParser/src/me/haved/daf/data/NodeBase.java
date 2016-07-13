package me.haved.daf.data;

import me.haved.daf.lexer.tokens.Token;

public class NodeBase {
	//The end line is the actual line it ends on
	//The end col however is one past the actual last char
	protected Token start, end;
	
	public int getLine() {
		return start.getLine();
	}
	
	public int getCol() {
		return start.getCol();
	}
	
	public int getEndLine() {
		return end.getLine();
	}
	
	public int getEndCol() {
		return end.getEndCol();
	}
	
	public void setPosition(Token startToken, Token lastToken) {
		this.start = startToken;
		this.end = lastToken;
	}
}
