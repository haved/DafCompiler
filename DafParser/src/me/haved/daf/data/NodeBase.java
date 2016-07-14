package me.haved.daf.data;

import me.haved.daf.lexer.tokens.Token;

public class NodeBase {
	//The end line is the actual line it ends on
	//The end col however is one past the actual last char
	protected int line, col, endLine, endCol;
	
	public int getLine() {
		return line;
	}
	
	public int getCol() {
		return col;
	}
	
	public int getEndLine() {
		return endLine;
	}
	
	public int getEndCol() {
		return endCol;
	}
	
	public void setPosition(Token startToken, Token lastToken) {
		line = startToken.getLine();
		col = startToken.getCol();
		endLine = lastToken.getLine();
		endCol = lastToken.getEndCol();
	}
}
