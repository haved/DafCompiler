package me.haved.dafParser.semantic;

import java.util.ArrayList;

import me.haved.dafParser.lexical.Token;

public class TokenDigger {
	private ArrayList<Token> tokens;
	private int index;
	
	public TokenDigger(ArrayList<Token> tokens) {
		this.tokens = tokens;
		index = 0;
	}
	
	public Token currentAndAdvance() {
		if(index>=tokens.size())
			return tokens.get(index++);
		return null;
	}
	
	public Token current() {
		if(index<tokens.size())
			return tokens.get(index);
		return null;
	}
	
	public void advance() {
		index++;
	}
	
	public boolean hasMore() {
		return index < tokens.size();
	}
}
