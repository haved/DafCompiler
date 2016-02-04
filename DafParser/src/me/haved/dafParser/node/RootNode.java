package me.haved.dafParser.node;

import java.util.ArrayList;
import java.util.Collection;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenType;

import static me.haved.dafParser.LogHelper.*;

public class RootNode extends Node {

	private ArrayList<Definition> definitions;
	
	public RootNode() {
		definitions = new ArrayList<>();
	}
	
	@Override
	public int FillFromTokens(ArrayList<Token> tokens, int start) {
		if(start >= tokens.size()) {
			log(WARNING, "Node asked to look at tokens outside of list");
			return start;
		}
		
		int i = start;
		for(; i < tokens.size(); i++) {
			Token t = tokens.get(i);
			if(t.getType()==TokenType.DAF_CPP)
				definitions.add(new Inline(false, t.getText()));
			else if(t.getType()==TokenType.DAF_CPP)
				definitions.add(new Inline(false, t.getText()));
		}
		
		return i;
	}
	
	@Override
	public String getName() {
		return "Root Node";
	}

	@Override
	public String compileSubnodesToString() {
		StringBuilder builder = new StringBuilder("(RootNode, ");
		for(int i = 0; i < definitions.size(); i++) {
			if(i>0)
				builder.append(", ");
			builder.append(definitions.get(i).compileToString());
		}
		builder.append(" )");
		return builder.toString();
	}

	public Collection<Definition> getDefinitions() {
		return definitions;
	}
}
