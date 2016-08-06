package me.haved.daf.syxer;

import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.FunctionExpression;
import me.haved.daf.data.expression.InfixOperatorExpression;
import me.haved.daf.data.expression.NumberConstantExpression;
import me.haved.daf.data.expression.PrefixOperatorExpression;
import me.haved.daf.data.expression.VariableExpression;
import me.haved.daf.data.statement.FunctionParameter;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.Operators.InfixOperator;
import me.haved.daf.syxer.Operators.PostfixOperator;
import me.haved.daf.syxer.Operators.PrefixOperator;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class ExpressionParser {
	public static Expression parseExpression(TokenBufferer bufferer) {
		return parseSide(bufferer, 0);
	}
	
	public static Expression parseSide(TokenBufferer bufferer, int minimumPrecedence) {
		PrefixOperator preOp = Operators.parsePrefixOperator(bufferer); //Eats the op
		Expression LHS;
		if(preOp != null)
			LHS = new PrefixOperatorExpression(preOp, parseSide(bufferer, preOp.getPrecedence()+1));
		else
			LHS = parseLoneExpression(bufferer);
		while(true) {
			PostfixOperator op = Operators.findPostfixOperator(bufferer);
			if(op == null)
				break;
			if(op.getPrecedence() < minimumPrecedence)
				return LHS; //Let some previous call handle this one
			LHS = op.evaluate(LHS, bufferer); //Will eat the operator
		}
		while(true) {
			InfixOperator op = Operators.findInfixOperator(bufferer);
			if(op == null)
				return LHS;
			else if(op.getPrecedence() < minimumPrecedence)
				return LHS;
			bufferer.advance(); //Eat the op
			Expression RHS = parseSide(bufferer, op.getPrecedence()+1);
			LHS = new InfixOperatorExpression(LHS, op, RHS);
		}
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
	
	/*public static FunctionCall parseFunctionCall(Expression expression, TokenBufferer bufferer) {
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
	}*/
	
	public static Expression parseNumberConstant(TokenBufferer bufferer) {
		NumberConstantExpression exp = new NumberConstantExpression(bufferer.getCurrentToken());
		exp.setPosition(bufferer.getCurrentToken(), bufferer.getCurrentToken());
		bufferer.advance();
		return exp;
	}
}
