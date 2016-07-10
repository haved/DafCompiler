package me.haved.daf.syxer;

import me.haved.daf.data.FunctionCall;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class ExpressionParser {
	public static Expression parseExpression(TokenBufferer bufferer) {
		Token firstToken = bufferer.getCurrentToken(); //Has to be something
		
		while(true) {
			if(bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) { //A variable or procedure
				Expression a;
				String name = bufferer.getCurrentToken().getText();
				
				if(!advanceOrComplain(bufferer))
					return null;
				if(bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN)) { //A procedure call
					if(!advanceOrComplain(bufferer))
						return null;
					if(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
						log(bufferer.getCurrentToken(), ERROR, "For the time being, only empty parameter lists are allowed");
					}
					if(!advanceOrComplain(bufferer))
						return null;
					a = new FunctionCall(name, null);
				} else
					a = new VariableExpression(name);
				
				//Past variable / function
				
			}
			else {
				
			}
		}
	}
	
	private static boolean advanceOrComplain(TokenBufferer bufferer) {
		if(!bufferer.advance()) {
			log(bufferer.getLastToken(), ERROR, "Expected a '%s' before end of file!", TokenType.SEMICOLON);
			return false;
		}
		return true;
	}
}
