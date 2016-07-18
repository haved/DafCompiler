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
	
	private static final String LET_DURING = "a let statement";
	public static Let parseLetStatement(TokenBufferer bufferer, boolean pub) {
		boolean uncertain = false;
		boolean mut = false;
		boolean letMet = false;
		
		bufferer.advance(); //Eat the 'let'
		
		while(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			if(bufferer.isCurrentTokenOfType(TokenType.LET)) {
				if(letMet) {
					log(bufferer.getCurrentToken(), ERROR, "Was met twice in the same definition. Aborting both.");
					SyntaxicParser.skipUntilSemicolon(bufferer);
					return null;
				}
				letMet = true;
				
			}
			if(bufferer.isCurrentTokenOfType(TokenType.UNCERTAIN)) {
				if(uncertain)
					log(bufferer.getCurrentToken(), WARNING, "Let declared as uncertain twice");
				uncertain = true;
				bufferer.advance();
			} else if(bufferer.isCurrentTokenOfType(TokenType.MUT)) {
				if(mut)
					log(bufferer.getCurrentToken(), WARNING, "Let declared as mutable twice");
				mut = true;
				if(!advanceOrComplain(bufferer, LET_DURING))
						return null;
			} else {
				log(bufferer.getCurrentToken(), ERROR, "Expected an identifer", TokenType.LET, TokenType.UNCERTAIN); //Uncertain is the only other way
				SyntaxicParser.skipUntilSemicolon(bufferer);
				return null;
			}
		}
		
		if(!letMet) {
			log();
			
		}
		
		if(uncertain && !mut)
			log(bufferer.getCurrentToken(), WARNING, "An uncertain %s statement has no reason to exist if not mutable", TokenType.LET);
		
		if(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			log(bufferer.getLastOrCurrent(), ERROR, "Expected an identifier after '%s'", TokenType.LET);
			SyntaxicParser.skipUntilSemicolon(bufferer);
		}
		
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
