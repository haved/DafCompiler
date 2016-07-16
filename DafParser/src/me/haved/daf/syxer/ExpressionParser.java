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
		return parseBinaryOpRHS(bufferer, LHS, 0);
	}
	
	/**
	 * Parses expressions with operands between them until the token buffer runs out of operators
	 * 
	 * @param bufferer the token source
	 * @param LHS the left hand side of the operand currently the current token
	 * @param originOpLevel the precedence of the operand before the LHS
	 * @return The Expression all the way from the LHS to the end of the operators (that have a higher level than the originOpLevel)
	 */
	public static Expression parseBinaryOpRHS(TokenBufferer bufferer, Expression LHS, int originOpLevel) {
		InfixOperator op = Operators.findInfixOperator(bufferer.getCurrentToken().getType()); //Just get the initial operator
		if(op == null)
			return LHS;
		int opLevel = op.getLevel();
		
		while(true) {
			InfixOperator prevOp = op;
			int prevOpLevel = opLevel;
			
			bufferer.advance(); //Eat the operator
			Expression RHS = parsePrimary(bufferer);
			op = Operators.findInfixOperator(bufferer.getCurrentToken().getType());
			if(op == null)
				return new InfixOperatorExpression(LHS, prevOp, RHS); //We are done!
			
			opLevel = op.getLevel();
			
			if(opLevel < originOpLevel) { //Say a - b * c == d where prevOp is * and op is ==. We can't eat the == because of the -
				return new InfixOperatorExpression(LHS, prevOp, RHS);
			}
			if(opLevel > prevOpLevel) { //Say a + b * c where + is prevOp and * is op
				RHS = parseBinaryOpRHS(bufferer, RHS, prevOpLevel);
				op = Operators.findInfixOperator(bufferer.getCurrentToken().getType());
				if(op == null)
					return new InfixOperatorExpression(LHS, prevOp, RHS);
				opLevel = op.getLevel();
			}
			
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
