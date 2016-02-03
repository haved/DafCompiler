package me.haved.dafParser.semantic;

import java.util.ArrayList;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.node.RootNode;

import static me.haved.dafParser.LogHelper.*;

public class SemanticParser {
	
	private ArrayList<Token> tokens;
	private RootNode node;
	
	public SemanticParser(ArrayList<Token> tokens) {
		this.tokens = tokens;
	}
	
	public void parse() {
		logAssert(tokens != null, "The tokens passed to the Semantic Parser were null");
		log(MESSAGE, "Starting semantic parsing with %d tokens", tokens.size());
		node = new RootNode();
		int end = node.FillFromTokens(tokens, 0);
		if(end < tokens.size()) {
			log(WARNING, "It would seem not all tokens were read!");
		}
		
		log(MESSAGE, node.compileSubnodesToString());
		
		terminateIfErrorsLogged();
		log(MESSAGE, "Finished semantic parsing");
	}
	
	public RootNode getRootNode() {
		logAssert(node != null, "The Semantic Parsers hasn't parsed the file yet!");
		return node;
	}
}
