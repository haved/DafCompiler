package me.haved.daf.syxer;

import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.InfixOperatorExpression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.data.statement.FunctionCall;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.Operators.InfixOperator;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class ExpressionParser {
	public static Expression parseExpression(TokenBufferer bufferer) {
		Expression LHS = parsePrimary(bufferer);
		if(LHS == null)
			return null;
		return parseBinaryOpRHS(bufferer, LHS);
	}
	
	//Higher level means higher priority => + has a lower level than *
	//When called, LHS is the LHS of the potential operator that is the current token
	//The operator value is checked = op
	//You parse the RHS as well = middle
	//The next operator value is checked, but the operator is NOT eaten
	//If the next operator level is higher, start with the middle as the LHS and repeat, to get the RHS, return LHS (op) parseBinaryOpRHS(...)
	//Else, set the LHS to LHS (op) middle, keep going
	public static Expression parseBinaryOpRHS(TokenBufferer bufferer, Expression LHS) {
		InfixOperator op = Operators.findInfixOperator(bufferer.getCurrentToken().getType());
		if(op == null)
			return LHS;
		int opLevel = op.getLevel();
		
		while(true) {
			bufferer.advance(); //Eat the operator
			InfixOperator prevOp = op;
			int prevLevel = opLevel;
			
			Expression RHS = parsePrimary(bufferer);
			if(RHS == null) {
				log(bufferer.getCurrentToken(), ERROR, "No RHS was found to the operator %s", op);
				return null;
			}
			if(!bufferer.hasCurrentToken()) {
				log(bufferer.getLastToken(), ERROR, "Expected something after the RHS!", op);
				return null;
			}
			op = Operators.findInfixOperator(bufferer.getCurrentToken().getType()); //Don't eat it
			
			if(op == null)
				return new InfixOperatorExpression(LHS, prevOp, RHS);
			
			opLevel = op.getLevel();
			
			if(opLevel > prevLevel) // example:  LHS + RHS *
				return new InfixOperatorExpression(LHS, prevOp, parseBinaryOpRHS(bufferer, RHS));
			else //example:  LHS * RHS +
				LHS = new InfixOperatorExpression(LHS, prevOp, RHS);
		}
	}
	
	public static Expression parsePrimary(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) //Precisely why we should have ended on EOF_token
			return null;
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
