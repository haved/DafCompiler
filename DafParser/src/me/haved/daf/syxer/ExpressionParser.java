package me.haved.daf.syxer;

import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.InfixOperatorExpression;
import me.haved.daf.data.expression.NumberConstantExpression;
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
	 * @param beforeLHS the precedence of the operand before the LHS
	 * @return The Expression all the way from the LHS to the end of the operators (that have a higher level than the originOpLevel)
	 */
	public static Expression parseBinaryOpRHS(TokenBufferer bufferer, Expression LHS, int beforeLHS) {
		InfixOperator afterRHS = Operators.findInfixOperator(bufferer.getCurrentToken().getType()); //Just get the initial operator
		if(afterRHS == null) //If there was no operator, return the expression
			return LHS;
		//At this point, "after RHS is a lie, but it's be once we're in the while loop"
		
		while(true) {
			InfixOperator afterLHS = afterRHS; //RHS was merged into LHS, so the new afterLHS is the old afterRHS
			
			bufferer.advance(); //Eat the operator
			Expression RHS = parsePrimary(bufferer);
			
			afterRHS = Operators.findInfixOperator(bufferer.getCurrentToken().getType());
			if(afterRHS == null)
				return new InfixOperatorExpression(LHS, afterLHS, RHS); //We are done!
			
			if(afterRHS.getLevel() <= beforeLHS) //Say a - b * c == d where afterLHS is * and afterRHS is ==. We can't eat the == because of the -
				return new InfixOperatorExpression(LHS, afterLHS, RHS);
				
			if(afterRHS.getLevel() > afterLHS.getLevel()) { //Say a + b * c where + is afterLHS and * is op
				RHS = parseBinaryOpRHS(bufferer, RHS, afterLHS.getLevel());
				afterRHS = Operators.findInfixOperator(bufferer.getCurrentToken().getType()); //Update the operator after RHS
				if(afterRHS == null) //If the RHS is the end, just return now
					return new InfixOperatorExpression(LHS, afterLHS, RHS);
			}
			
			LHS = new InfixOperatorExpression(LHS, afterLHS, RHS);
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
		case NUMBER_LITERAL:
			return parseNumberConstant(bufferer);
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
			if(!bufferer.isCurrentTokenOfType(TokenType.COMMA))
				log(bufferer.getCurrentToken(), ERROR, "Expected comma or ')' after function parameter");
			if(!bufferer.advance()) { //Just to keep this show from running forever.
				log(bufferer.getLastToken(), ERROR, "Expected ) to end function call. Not EOF!");
			}
		} while(true); //I dunno
		
		bufferer.advance(); //Eat )
		
		return new FunctionCall(idName, params.toArray(new Expression[params.size()]));
	}
	
	public static Expression parseNumberConstant(TokenBufferer bufferer) {
		NumberConstantExpression exp = new NumberConstantExpression(bufferer.getCurrentToken());
		bufferer.advance();
		return exp;
	}
}
