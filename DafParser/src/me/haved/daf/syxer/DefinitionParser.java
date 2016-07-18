package me.haved.daf.syxer;

import me.haved.daf.data.definition.Definition;
import me.haved.daf.data.definition.Let;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class DefinitionParser {
	public static Definition parseDefinition(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken())
			return null;
		
		boolean pub = false;
		
		TokenType type = bufferer.getCurrentToken().getType();
		if(type == TokenType.PUB) {
			pub = true;
			if(!advanceOrComplain(bufferer, "a definition"))
				return null;	
			type = bufferer.getCurrentToken().getType();
		}
		
		switch(type) {
		case LET: return parseLetStatement(bufferer, pub);
		case UNCERTAIN: return parseLetStatement(bufferer, pub);
		default: break;
		}
		
		log(bufferer.getCurrentToken(), ERROR, "Couldn't parse definition from token");
		return null;
	}
	
	public static Let parseLetStatement(TokenBufferer bufferer, boolean pub) {
		boolean uncertain = false;
		
		TokenType type = bufferer.getCurrentToken().getType();
		if(type == TokenType.UNCERTAIN) {
			uncertain = true;
			if(!advanceOrComplain(bufferer, "a let statement"))
				return null;
			type = bufferer.getCurrentToken().getType();
		}
		
		if(type != TokenType.LET) {
			log(bufferer.getCurrentToken(), ERROR, "Expected %s after %s", TokenType.LET);
			SyntaxicParser.skipUntilSemicolon(bufferer, "a let statement");
			return null;
		}
		
		bufferer.advance(); //Eat the 'let'
		
		return null;
	}
	
	private static boolean advanceOrComplain(TokenBufferer bufferer, String during) {
		if(!bufferer.advance()) {
			log(bufferer.getLastToken(), ERROR, "Expected more when parsing %s. Not EOF!", during);
			return false;
		}
		return true;
	}
}
