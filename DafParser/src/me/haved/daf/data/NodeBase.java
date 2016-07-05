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
	
	public void setPosition(int line, int col, int endLine, int endCol) {
		this.line = line;
		this.col = col;
		this.endLine = endLine;
		this.endCol = endCol;
	}
	
	public void setPosition(int line, int col, int length) {
		this.line = line;
		this.col = col;
		this.endLine = line;
		this.endCol = col + length;
	}
	
	public void setPosition(Token startToken, Token lastToken) {
		this.line = startToken.getLine();
		this.col = startToken.getCol();
		this.endLine = lastToken.getLine();
		this.endCol = lastToken.getEndCol();
	}
}
