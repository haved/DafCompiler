package me.haved.daf.syxer;

import me.haved.daf.data.statement.Statement;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

public class StatementParser {
	public static Statement parseStatement(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a statement! Got EOF");
			return null;
		}
		
		Token firstToken = bufferer.getCurrentToken();
		
		return null;
	}
}
