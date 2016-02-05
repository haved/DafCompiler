package me.haved.dafParser.semantic;

import java.util.ArrayList;

import me.haved.dafParser.lexical.Token;

public class TokenPosition {
	public ArrayList<Token> tokens;
	public int position = 0;
	
	public TokenPosition(ArrayList<Token> tokens) {
		this.tokens = tokens;
	}
	
	public int count() {
		return tokens.size();
	}
	
	public Token get(int index) {
		return tokens.get(index);
	}
	
	public boolean hasMore() {
		return position < tokens.size();
	}
	
	public void next() {
		position++;
	}
	
	public Token current() {
		return get(position);
	}
}
