package me.haved.daf.syxer;

import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.data.statement.FunctionCall;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class ExpressionParser {
	public static Expression parseExpression(TokenBufferer bufferer) {
		switch(bufferer.getCurrentToken().getType()) {
		default:
			break;
		case IDENTIFER:
			return parseIdentifierExpression(bufferer);
		case LEFT_PAREN:
			return parseParentheses(bufferer);
		}
		
		log(bufferer.getCurrentToken(), ERROR, "Expected an expression!"); return null;
	}
	
	private static Expression parseParentheses(TokenBufferer bufferer) {
		bufferer.advance();
		Expression expression = parseExpression(bufferer);
		bufferer.advance();
		if(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			log(bufferer.getCurrentToken(), ERROR, "Expected a closing ')'"); 
			//We know the current token is the last token, even if we're out of tokens
		}
		return expression;
	}

	public static Expression parseIdentifierExpression(TokenBufferer bufferer) {
		String id = bufferer.getCurrentToken().getText();
		if(!bufferer.advance()) { log(bufferer.getLastToken(), ERROR, "Expected something after expression before EOF"); return null; }
		
		if(!bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN)) //Not a function call
			return new VariableExpression(id);
		
		if(!bufferer.advance()) { log(bufferer.getLastToken(), ERROR, "Expected ')' before EOF"); return null; }
		if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			bufferer.advance();
			return new FunctionCall(id, null);
		}
		
		ArrayList<Expression> params = new ArrayList<>();
		do {
			Expression param = parseExpression(bufferer);
			if(param==null)
				return null;
			params.add(param);
			if(bufferer.isCurrentTokenOfType(TokenType.COLON))
				bufferer.advance();
			else if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN))
				break;
			else
				log(bufferer.getCurrentToken(), ERROR, "Expected comma or ')' after function parameter");
		} while(true); //I dunno
		
		if(!bufferer.advance()) { log(bufferer.getLastToken(), ERROR, "Expected something after function call... Not EOF!"); return null; }
		
		return new FunctionCall(id, params.toArray(new Expression[params.size()]));
	}
}
