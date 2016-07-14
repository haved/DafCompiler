package me.haved.daf.syxer;

import me.haved.daf.data.FunctionCall;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.ExpressionInfixOperator;
import me.haved.daf.data.expression.NumberExpression;
import me.haved.daf.data.expression.OperatorExpression;
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
			Token token = bufferer.getCurrentToken();
			log(token, DEBUG, "eyyy");
			
			//Check for start operator or (
			
			Expression a;
			if(token.getType() == TokenType.IDENTIFER) { //A variable or procedure
				String name = bufferer.getCurrentToken().getText();
				
				if(!advanceOrComplain(bufferer))
					return null;
				if(bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN)) { //A procedure call
					while(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN) && bufferer.advance());
					a = new FunctionCall(name, null).setFunctionCallPosition(token, bufferer.getCurrentToken());
					if(!advanceOrComplain(bufferer))
						return null;
				} else
					a = new VariableExpression(name).setVariablePosition(token); //TODO: Set position
				
				//Past variable / function
			}
			else if(token.getType() == TokenType.NUMBER_LITTERAL) {
				a = new NumberExpression(token).setPosition(token);
				bufferer.advance();
			} else if(token.getType() == TokenType.STRING_LITTERAL) {
				a = new StringExpression(token.getText()).setPosition(token);
				bufferer.advance();
			} else {
				log(bufferer.getCurrentToken(), ERROR, "Expected an expression!");
				bufferer.advance();
				return null;
			}
			
			token = bufferer.getCurrentToken(); //Past the expression
			TokenType type = token.getType();
			//Infix operators, ;, ',', )
			
			if(type == TokenType.SEMICOLON || type == TokenType.COMMA || type == TokenType.RIGHT_PAREN) {
				return a;
			}
			
			for(ExpressionInfixOperator op : ExpressionInfixOperator.values()) {
				if(op.matchesToken(type)) {
					//infix = new OperatorExpression(a, null, op);
					break;
				}
			}
			
			bufferer.advance(); //I dunno
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
