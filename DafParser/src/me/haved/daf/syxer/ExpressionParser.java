package me.haved.daf.syxer;

import me.haved.daf.data.FunctionCall;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.ExpressionInfixOperator;
import me.haved.daf.data.expression.NumberExpression;
import me.haved.daf.data.expression.StringExpression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class ExpressionParser {
	public static Expression parseExpression(TokenBufferer bufferer) {
		Token firstToken = bufferer.getCurrentToken(); //Has to be something
		
		ExpressionInfixOperator infix = null;
		
		while(true) {
			Token startToken = bufferer.getCurrentToken();
			
			//Check for start operator or (
			
			Expression a;
			if(startToken.getType() == TokenType.IDENTIFER) { //A variable or procedure
				String name = bufferer.getCurrentToken().getText();
				
				if(!advanceOrComplain(bufferer))
					return null;
				if(bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN)) { //A procedure call
					while(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN) && bufferer.advance());
					a = new FunctionCall(name, null).setFunctionCallPosition(startToken, bufferer.getCurrentToken());
					if(!advanceOrComplain(bufferer))
						return null;
				} else
					a = new VariableExpression(name).setVariablePosition(startToken); //TODO: Set position
				
				//Past variable / function
			}
			else if(startToken.getType() == TokenType.NUMBER_LITTERAL) {
				a = new NumberExpression(startToken).setPosition(startToken);
			} else if(startToken.getType() == TokenType.STRING_LITTERAL) {
				a = new StringExpression(startToken.getText()).setPosition(startToken);
			}
			
			bufferer.advance();
			
			//Infix operators, ;, ',', (
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
