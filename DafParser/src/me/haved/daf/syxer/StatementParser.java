package me.haved.daf.syxer;

import me.haved.daf.data.definition.Def;
import me.haved.daf.data.definition.Let;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.statement.ScopeStatement;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class StatementParser {
	public static Statement parseStatement(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a statement! Got EOF");
			return null;
		}
		
		Token firstToken = bufferer.getCurrentToken();
		TokenType type = firstToken.getType();
		if(type == TokenType.SCOPE_START)
			return parseScope(bufferer);
		if(type == TokenType.SEMICOLON) {
			bufferer.advance(); //Eat the semicolon
			return null; //No statement here (allows for ;;;;;;)
		}
		
		//control statements go here, and are not followed by semi-colon
		
		
		//If you're here, you want a semicolon behind the statement
		
		Statement output = null;
		boolean wrong = false;
		
		switch(type) {
		case LET: output = parseLetStatement(bufferer); break;
		case DEF: output = parseDefStatement(bufferer); break;
		default: wrong = true; break;
		}
		
		if(wrong) {
			//Maybe an expression? Still expecting a ; afterwards
			Token expressionStart = bufferer.getCurrentToken();
			Expression expression = ExpressionParser.parseExpression(bufferer);
			if(expression != null && expression instanceof Statement && ((Statement)expression).isValidStatement()) {
				output = (Statement) expression;
				wrong = false;
			}
			else {
				if(expression != null)
					log(expressionStart, ERROR, "Expected a statement, got the expression: '%s'. "
							+ "Skipping till semicolon or scope change", expression.getSignature());
				else
					log(expressionStart, ERROR, "Expected a statement. Skipping till semicolon or scope change");
				while(bufferer.hasCurrentToken()) {
					if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
						bufferer.advance();
						return null;
					} else if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_END) || bufferer.isCurrentTokenOfType(TokenType.SCOPE_START)) {
						return null;
					}
					bufferer.advance();
				}
			}
		}
		
		if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON))
			bufferer.advance();
		else
			log(bufferer.getLastOrCurrent(), ERROR, "Expected a semicolon after statement");
		
		return output;
	}
	
	private static ScopeStatement parseScope(TokenBufferer bufferer) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.SCOPE_START));
		Token firstToken = bufferer.getCurrentToken(); // The '{'
		bufferer.advance(); //Eat '{'
		ArrayList<Statement> statements = new ArrayList<>();
		while(true) {
			if(!bufferer.hasCurrentToken()) {
				log(firstToken, ERROR, "Scope starting here never closed (or some skipping occured)");
				return null;
			}
			else if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_END))
				break;
			
			Statement statement = parseStatement(bufferer);
			if(statement != null)
				statements.add(statement); //The statement ends after itself
		}
		
		ScopeStatement output = new ScopeStatement(statements.isEmpty() ? null : statements.toArray(new Statement[statements.size()]));
		output.setPosition(firstToken, bufferer.getCurrentToken());
		bufferer.advance(); //Eat the '}'
		return output;
	}

	/**
	 * Reads from the bufferer and parses a Let statement. Leaves the bufferer right at the semi-colon / after the expression
	 * 
	 * @param bufferer the token bufferer
	 * @return the let statement
	 */
	private static Let parseLetStatement(TokenBufferer bufferer) {
		return DefinitionParser.parseLetStatement(bufferer, false);
	}
	
	// Same as parseLetStatement, just for def
	private static Def parseDefStatement(TokenBufferer bufferer) {
		return DefinitionParser.parseDefStatement(bufferer, false);
	}
}
