package me.haved.daf.syxer;

import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.FunctionExpression;
import me.haved.daf.data.expression.InfixOperatorExpression;
import me.haved.daf.data.expression.NumberConstantExpression;
import me.haved.daf.data.expression.PrefixOperatorExpression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.data.statement.FunctionCall;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.Operators.InfixOperator;
import me.haved.daf.syxer.Operators.PrefixOperator;

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
		if(!bufferer.hasCurrentToken())
			return LHS;
		InfixOperator afterRHS = Operators.findInfixOperator(bufferer.getCurrentToken().getType()); //Just get the operator
		if(afterRHS == null) //If there was no operator, return the expression
			return LHS;
		//At this point, "after RHS is a lie, but it's true once we're in the while loop"
		
		while(true) {
			InfixOperator afterLHS = afterRHS; //RHS was merged into LHS, so the new afterLHS is the old afterRHS
			
			bufferer.advance(); //Eat the operator
			Expression RHS = parsePrimary(bufferer);
			
			afterRHS = Operators.findInfixOperator(bufferer.getCurrentToken().getType());
			if(afterRHS == null)
				return new InfixOperatorExpression(LHS, afterLHS, RHS); //We are done! Just merge LHS and RHS
			
			if(afterRHS.getLevel() <= beforeLHS) //Say a - b * c == d where afterLHS is * and afterRHS is ==. We can't eat the == because of the -
				return new InfixOperatorExpression(LHS, afterLHS, RHS);
				
			if(afterRHS.getLevel() > afterLHS.getLevel()) { //Say a + b * c where + is afterLHS and * is op
				RHS = parseBinaryOpRHS(bufferer, RHS, afterLHS.getLevel());
				afterRHS = Operators.findInfixOperator(bufferer.getCurrentToken().getType()); //Update the operator after RHS
				if(afterRHS == null) //If the RHS is the end, just return now
					return new InfixOperatorExpression(LHS, afterLHS, RHS);
			}
			
			LHS = new InfixOperatorExpression(LHS, afterLHS, RHS); //Merge LHS and RHS
		}
	}
	
	/**
	 * 
	 * @param bufferer
	 * @return an expression with only pre- and post-fix operators
	 */
	public static Expression parsePrimary(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) //Precisely why we should have ended on EOF_tokens
			return null;
		Token firstToken = bufferer.getCurrentToken();
		PrefixOperator op = Operators.parsePrefixOperator(bufferer);
		if(op != null) {
			if(!bufferer.hasCurrentToken()) {
				log(bufferer.getLastToken(), ERROR, "Expected an expression after %s, not EOF", op);
				return null;
			}
			Expression exp = parsePrimary(bufferer);
			if(exp == null)
				return null; //Errors already printed
			return new PrefixOperatorExpression(op, exp).setStart(firstToken);
		}
		return parseWithPostfix(parseLoneExpression(bufferer), bufferer);
	}
	
	public static Expression parseWithPostfix(Expression expression, TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken())
			return expression;
		
		if(bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN))
			return parseWithPostfix(parseFunctionCall(expression, bufferer), bufferer);
		
		return expression;
	}
	
	public static Expression parseLoneExpression(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken())
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
		
		log(bufferer.getCurrentToken(), ERROR, "Expected an expression!");
		return null;
	}
	
	private static Expression parseParentheses(TokenBufferer bufferer) {
		Token firstToken = bufferer.getCurrentToken();
		bufferer.advance(); //Eat the (
		if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) //If we have a '()', parse as a function from ')'
			return parseFunction(bufferer, null).setStart(firstToken);
		
		else if(bufferer.isCurrentTokenOfType(TokenType.getAddressType()) 
				&& bufferer.hasLookaheadToken() && bufferer.getLookaheadToken().getType() == TokenType.MOVE) //If we have '(&move', parse func from &
				return parseFunction(bufferer, null).setStart(firstToken);
		
		Expression expression = parseExpression(bufferer); // We parse the expression past '('
		if(expression != null) { //If no expression was parsed, skip the next ')' and return null
			if(bufferer.isCurrentTokenOfType(TokenType.COLON)) //If we found a colon after the expression, the expression is actually a parameter name
				return parseFunction(bufferer, expression).setStart(firstToken);
			
			if(bufferer.hasCurrentToken())
				expression.setPosition(firstToken, bufferer.getCurrentToken());
			
			if(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) { //If it is a normal parenthesized expression, expect ')'
				log(bufferer.getLastOrCurrent(), ERROR, "Expected a closing ')'"); 
				bufferer.skipUntilTokenType(TokenType.RIGHT_PAREN); 
			}
		} else {
			bufferer.skipUntilTokenType(TokenType.RIGHT_PAREN); //No expression means we return null, but skip to, then past ')'
		}
		bufferer.advance(); //Eat the ')'
		return expression;
	}

	private static FunctionExpression parseFunction(TokenBufferer bufferer, Expression firstParam) {
		FunctionParameter[] params = parseFunctionParameters(bufferer, firstParam);
		if(params == null)
			bufferer.skipUntilTokenType(TokenType.RIGHT_PAREN); //Not very good error handling
		logAssert(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN));
		bufferer.advance(); //Eat the ')'
		
		//Parse return type
		Type returnType = null;
		if(bufferer.isCurrentTokenOfType(TokenType.COLON)) {
			bufferer.advance(); //Eat the ':'
			returnType = TypeParser.parseType(bufferer);
		}
		
		//Parse statement
		Statement statement = StatementParser.parseStatement(bufferer);
		FunctionExpression output = new FunctionExpression(params, returnType, statement);
		return output;
	}
	
	private static FunctionParameter[] parseFunctionParameters(TokenBufferer bufferer, Expression firstParam) {
		if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			logAssert(firstParam == null);
			return null;
		}
		ArrayList<FunctionParameter> params = new ArrayList<>();
		while(true) {
			int refType = FunctionParameter.NOT_A_REF;
			String paramName = null;
			
			if(params.size() == 0 && firstParam != null) {
				if(firstParam instanceof VariableExpression) {
					paramName = ((VariableExpression)firstParam).getName();
				} else if(firstParam instanceof PrefixOperatorExpression) {
					PrefixOperatorExpression param = (PrefixOperatorExpression)firstParam;
					PrefixOperator op = param.getOperator();
					if(op == PrefixOperator.MUT_ADDRESS)
						refType = FunctionParameter.MUTBL_REF;
					else if(op == PrefixOperator.ADDRESS)
						refType = FunctionParameter.CONST_REF;
					else {
						log(bufferer.getCurrentToken().getFile(), firstParam.getLine(), firstParam.getCol(), 
								ERROR, "The operator '%s' before a parameter was not recognized!", op.getName());
					}
					if(param.getBaseExpression() instanceof VariableExpression) {
						paramName = ((VariableExpression)param.getBaseExpression()).getName();
					} else {
						log(bufferer.getCurrentToken().getFile(), param.getBaseExpression().getLine(), param.getBaseExpression().getCol(), 
								ERROR, "Expected a parameter name after refrence type!");
					}
				} else {
					log(bufferer.getCurrentToken().getFile(), firstParam.getLine(), firstParam.getCol(),
							ERROR, "Expected a parameter name and a refrence");
				}
			} else {
				if(bufferer.isCurrentTokenOfType(TokenType.getAddressType())) {
					bufferer.advance();
					if(bufferer.isCurrentTokenOfType(TokenType.MUT)) {
						refType = FunctionParameter.MUTBL_REF;
						bufferer.advance();
					}
					else if(bufferer.isCurrentTokenOfType(TokenType.MOVE)) {
						refType = FunctionParameter.MOVES_REF;
						bufferer.advance();
					}
					else {
						refType = FunctionParameter.CONST_REF;
					}
				}
				//Expect name now!
				if(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
					log(bufferer.getLastOrCurrent(), ERROR, "Expected parameter name in function value!");
				}
				paramName = bufferer.getCurrentToken().getText();
				bufferer.advance();
			}
			
			logAssert(paramName != null);
			
			if(!bufferer.isCurrentTokenOfType(TokenType.COLON)) {
				log(bufferer.getLastOrCurrent(), ERROR, "Expected ':' after parameter name in function value");
			}
			bufferer.advance(); //Eat the ':'
			
			Type paramType = TypeParser.parseType(bufferer);
			terminateIfErrorsOccured();
			params.add(new FunctionParameter(refType, paramName, paramType));
			if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN))
				break;
			else if(!bufferer.hasCurrentToken()) {
				log(bufferer.getLastOrCurrent(), ERROR, "Ran out of tokens while parsing function parameters (expression)");
				return null;
			} else if(!bufferer.isCurrentTokenOfType(TokenType.COMMA)) {
				log(bufferer.getCurrentToken(), ERROR, "Expected a comma between function parameters, or ) to end the parameter list");
			}
			bufferer.advance(); //Eat ','
		}
		return params.toArray(new FunctionParameter[params.size()]);
	}
	
	public static Expression parseIdentifierExpression(TokenBufferer bufferer) {
		Token firstToken = bufferer.getCurrentToken();
		String idName = bufferer.getCurrentToken().getText();
		bufferer.advance(); //'Eat the identifier' as the llvm tutorial says
		return new VariableExpression(idName).setPosition(firstToken); //Simple variable. Just one token
	}
	
	public static FunctionCall parseFunctionCall(Expression expression, TokenBufferer bufferer) {
		if(!bufferer.advance()) //Eat '('
				{ log(bufferer.getLastToken(), ERROR, "Expected ')' before EOF"); return null; }
		if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			bufferer.advance(); //Eat ')'
			return new FunctionCall(expression, null);
		}
		
		ArrayList<Expression> params = new ArrayList<>();
		do {
			Expression param = ExpressionParser.parseExpression(bufferer);
			//if(param==null)
			//	return null;
			params.add(param);
			if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN))
				break;
			if(!bufferer.isCurrentTokenOfType(TokenType.COMMA))
				log(bufferer.getCurrentToken(), ERROR, "Expected comma or ')' after function parameter");
			if(!bufferer.advance()) { //Just to keep this show from running forever.
				log(bufferer.getLastToken(), ERROR, "Expected ')' to end function call. Not EOF!");
			}
		} while(true); //I dunno
		
		FunctionCall output = new FunctionCall(expression, params.toArray(new Expression[params.size()]));
		output.setEnd(bufferer.getCurrentToken()); //Beginning gets set outside, since the name is passed
		bufferer.advance(); //Eat ')'
		return output;
	}
	
	public static Expression parseNumberConstant(TokenBufferer bufferer) {
		NumberConstantExpression exp = new NumberConstantExpression(bufferer.getCurrentToken());
		exp.setPosition(bufferer.getCurrentToken(), bufferer.getCurrentToken());
		bufferer.advance();
		return exp;
	}
}
