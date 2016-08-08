package me.haved.daf.syxer;

import me.haved.daf.data.definition.Def;
import me.haved.daf.data.definition.Let;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.statement.IfStatement;
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
		switch(type) {
		case SCOPE_START: return parseScope(bufferer);
		case SEMICOLON: bufferer.advance(); return null;
		default: break;
		}
		
		//control statements go here, and are not followed by semi-colon
		switch(type) {
		case IF: return parseIfStatement(bufferer);
		default: break;
		}
		
		//If you're here, you want a semicolon behind the statement
		
		Statement output = null;
		boolean wrong = false;
		
		switch(type) {
		case LET: output = parseLetStatement(bufferer); break;
		case DEF: output = parseDefStatement(bufferer); break;
		default: wrong = true; break;
		}
		if(wrong) //Try parsing an expression instead
			output = parseExpressionAsStatement(bufferer);
		
		return assureStatementEnd(bufferer, output);
	}
	
	private static Statement parseExpressionAsStatement(TokenBufferer bufferer) {
		Token expressionStart = bufferer.getCurrentToken();
		Expression expression = ExpressionParser.tryParseExpression(bufferer);
		if(expression==null) {
			log(expressionStart, ERROR, "Expected a statement");
			return null;
		}
		if(expression instanceof Statement) {
			Statement statement = (Statement) expression;
			if(statement.isValidStatement()) {
				return statement;
			}
		}
		log(expressionStart, ERROR, "Expacted a statement. Got the expression '%s'", expression.getSignature());
		return null;
	}
	/** 
	 * Makes sure the TokenBufferer ends right after a semi-colon.
	 * If the output is null, skips past first ';', or until '{' or '}'
	 * If a semi-colon isn't found, nothing happens.
	 * If you both have a bad statement and are missing a semicolon, the next statement will be skipped :(
	 * 
	 * @param bufferer the token bufferer
	 * @param output the statement before the wanted end
	 * @return output
	 */
	private static Statement assureStatementEnd(TokenBufferer bufferer, Statement output) {
		if(output == null) { //Skip until semicolon or scope end
			if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON))
				bufferer.advance(); //Eat ';'
			else if(bufferer.hasCurrentToken()) {
				Token firstToken = bufferer.getCurrentToken();
				StringBuilder skipped = new StringBuilder();
				int prevLine=firstToken.getLine(), prevColEnd=firstToken.getEndCol();
				while(bufferer.hasCurrentToken()) {
					Token token = bufferer.getCurrentToken();
					for(int i = prevLine; i < token.getLine(); i++) {
						prevColEnd = 1;
						skipped.append("\n");
					}
					for(int i = prevColEnd; i < token.getCol(); i++) {
						skipped.append(" ");
					}
					skipped.append(token.getText());
					prevLine = token.getLine();
					prevColEnd = token.getEndCol();
					if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
						bufferer.advance(); //Eat ';'
						break;
					} else if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_END) || bufferer.isCurrentTokenOfType(TokenType.SCOPE_START))
						break;
					bufferer.advance();
				}
				log(firstToken, ERROR, "Skipped '%s' to try to find the next statement", skipped.toString());
			}
		} else if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON))
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
	
	private static IfStatement parseIfStatement(TokenBufferer bufferer) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.IF));
		bufferer.advance(); //Eat 'if'
		Expression conditional = ExpressionParser.parseExpression(bufferer);
		Statement action = parseStatement(bufferer);
		if(!bufferer.isCurrentTokenOfType(TokenType.ELSE))
			return new IfStatement(conditional, action);
		bufferer.advance(); //Eat the 'else'
		return new IfStatement(conditional, action, parseStatement(bufferer));
	}
}
