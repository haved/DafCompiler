package me.haved.daf.syxer;

import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.data.statement.FunctionCall;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class ExpressionParser {
	public static Expression parseExpression(TokenBufferer bufferer) {
		Expression LHS = parseExpression(bufferer);
		if(LHS == null)
			return null;
		return parseBinaryOpRHS(0, LHS);
	}
	
	//Higher level means higher priority => + has a lower level than *
	public static Expression parseBinaryOpRHS(int operatorLevel, Expression LHS) {
		//Go as long as the next operator has a lower or same level as the previous operator
		//When the level increases, set RHS to the result of recursively calling this function
		
		return LHS;
	}
	
	public static Expression parsePrimary(TokenBufferer bufferer) {
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
		bufferer.advance(); //Eat the (
		Expression expression = parseExpression(bufferer);
		if(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			log(bufferer.getCurrentToken(), ERROR, "Expected a closing ')'"); 
			//We know the current token is the last token, even if we're out of tokens
		}
		bufferer.advance(); //Eat the )
		return expression;
	}

	public static Expression parseIdentifierExpression(TokenBufferer bufferer) {
		String idName = bufferer.getCurrentToken().getText();
		bufferer.advance(); //'Eat the identifier' as the llvm tutorial says
		
		if(!bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN)) //Simple variable
			return new VariableExpression(idName);
		
		if(!bufferer.advance()) //Eat (
				{ log(bufferer.getLastToken(), ERROR, "Expected ')' before EOF"); return null; }
		if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			bufferer.advance(); //Eat )
			return new FunctionCall(idName, null);
		}
		
		ArrayList<Expression> params = new ArrayList<>();
		do {
			Expression param = parseExpression(bufferer);
			if(param==null)
				return null;
			params.add(param);
			if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN))
				break;
			if(!bufferer.isCurrentTokenOfType(TokenType.COLON))
				log(bufferer.getCurrentToken(), ERROR, "Expected comma or ')' after function parameter");
			if(!bufferer.advance()) { //Just to keep this show from running forever.
				log(bufferer.getLastToken(), ERROR, "Expected ) to end function call. Not EOF!");
			}
		} while(true); //I dunno
		
		bufferer.advance(); //Eat )
		
		return new FunctionCall(idName, params.toArray(new Expression[params.size()]));
	}
}
