package me.haved.daf.syxer;

import me.haved.daf.data.definition.Def;
import me.haved.daf.data.definition.Definition;
import me.haved.daf.data.definition.Let;
import me.haved.daf.data.definition.ModuleDefinition;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.type.Type;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class DefinitionParser {
	public static Definition parseDefinition(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken())
			return null;
		
		boolean pub = false;
		
		TokenType type = bufferer.getCurrentToken().getType();
		if(type == TokenType.PUB) {
			pub = true;
			if(!bufferer.advance()) {
				log(bufferer.getLastToken(), ERROR, "Expected a defintion after '%s'", TokenType.PUB);
				return null;
			}
			type = bufferer.getCurrentToken().getType();
		}
		
		switch(type) {
		case MODULE: return parseModule(bufferer, pub);
		case LET: return parseLetStatement(bufferer, pub);
		case DEF: return parseDefStatement(bufferer, pub);
		default: break;
		}
		
		log(bufferer.getCurrentToken(), ERROR, "Expected a definition");
		return null;
	}
	
	public static ModuleDefinition parseModule(TokenBufferer bufferer, boolean pub) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.MODULE));
		Token firstToken = bufferer.getCurrentToken();
		bufferer.advance(); //Eat 'module'
		if(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			log(bufferer.getLastOrCurrent(), ERROR, "Expected an identifier after 'module'");
			return null;
		}
		String name = bufferer.getCurrentToken().getText();
		bufferer.advance(); //Eat the identifier;
		if(!bufferer.isCurrentTokenOfType(TokenType.SCOPE_START)) {
			log(bufferer.getLastOrCurrent(), ERROR, "Expected '{' after 'module %s'", name);
			return null;
		}
		bufferer.advance(); //Eat the '{'
		
		ArrayList<Definition> definitions = new ArrayList<>();
		SyntaxicParser.fillDefinitionList(definitions, bufferer);
		
		if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_END))
			bufferer.advance(); //Eat '}'
		else
			log(bufferer.getLastOrCurrent(), ERROR, "Expected the module '%s' to end with }", name);
		
		
		ModuleDefinition output = new ModuleDefinition(name, definitions.toArray(new Definition[definitions.size()]), pub);
		output.setPosition(firstToken, bufferer.getCurrentToken());
		
		return output;
	}
	
	public static Let parseLetStatement(TokenBufferer bufferer, boolean pub) {
		boolean uncertain = false;
		boolean mut = false;
		
		Token startToken = bufferer.getCurrentToken();
		bufferer.advance(); //Eat the 'let'
		
		while(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			if(bufferer.isCurrentTokenOfType(TokenType.UNCERTAIN)) {
				if(uncertain)
					log(bufferer.getCurrentToken(), WARNING, "Let declared as uncertain twice");
				uncertain = true;
			} else if(bufferer.isCurrentTokenOfType(TokenType.MUT)) {
				if(mut)
					log(bufferer.getCurrentToken(), WARNING, "Let declared as mutable twice");
				mut = true;
			} else { //elseOr would also be cool daf syntax
				log(bufferer.getCurrentToken(), ERROR, "Expected an identifer after %s", TokenType.LET);
				return null;
			}
			if(!bufferer.advance()) {
				log(bufferer.getLastToken(), ERROR, "Expected an idenftifier after %s. Not EOF", TokenType.LET);
				return null;
			}
		}
		
		if(uncertain && !mut)
			log(bufferer.getCurrentToken(), WARNING, "An uncertain %s statement has no reason to exist if not mutable", TokenType.LET);
		
		Token nameToken = bufferer.getCurrentToken();
		NameTypeExpr nte = parseNameTypeExpression(bufferer);
		if(nte==null)
			return null;
		
		if(nte.expression == null && !uncertain)
			log(nameToken, ERROR, "A %s statement without an initializer must be declared as uncertain.", TokenType.LET);
		else if(nte.expression != null && uncertain)
			log(nameToken, ERROR, "A %s statement can't be uncertain when it has an initializer!", TokenType.LET);
		
		Let output = new Let(nte.name, mut, nte.type, nte.expression, pub);
		output.setPosition(startToken, bufferer.getLastOrCurrent());
		return output;
	}
	
	public static Def parseDefStatement(TokenBufferer bufferer, boolean pub) {
		Token firstToken = bufferer.getCurrentToken();
		bufferer.advance(); //Eat the 'def'
		if(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			log(bufferer.getLastOrCurrent(), ERROR, "Expected an identifier after %s!", TokenType.DEF);
			return null;
		}
		
		NameTypeExpr nte = parseNameTypeExpression(bufferer);
		if(nte == null)
			return null;
		
		Def output = new Def(nte.name, nte.type, nte.expression, pub);
		if(bufferer.hasCurrentToken())
			output.setPosition(firstToken, bufferer.getCurrentToken());
		return output;
	}
	
	private static class NameTypeExpr {
		public final String name;
		public final Type type;
		public final Expression expression;
		
		public NameTypeExpr(String name, Type type, Expression expression) {
			this.name = name;
			this.type = type;
			this.expression = expression;
		}
	}
	
	private static NameTypeExpr parseNameTypeExpression(TokenBufferer bufferer) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.IDENTIFER));
		String identifier = bufferer.getCurrentToken().getText();
		bufferer.advance(); //Eat the identifier
		
		//either : or :=
		
		Type type;
		boolean autoType = false;
		if(bufferer.isCurrentTokenOfType(TokenType.COLON)) {
			bufferer.advance(); //Past ':'
			type = TypeParser.parseType(bufferer);
		} else if(bufferer.isCurrentTokenOfType(TokenType.COLON_ASSIGN)) {
			autoType = true;
			type = null;
		} else {
			log(bufferer.getLastOrCurrent(), ERROR, "Expected '%s' or '%s' after the identifer '%s' in a let statement", 
					TokenType.COLON, TokenType.COLON_ASSIGN, identifier);
			return null;
		}
		
		//Either =, := or ;
		
		if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			logAssert(!autoType); //If autoType, it has to be :=
			return new NameTypeExpr(identifier, type, null);
		}
		
		if(!bufferer.isCurrentTokenOfType(TokenType.COLON_ASSIGN) && !bufferer.isCurrentTokenOfType(TokenType.ASSIGN)) {
			log(bufferer.getLastOrCurrent(), ERROR, "Expected '%s', '%s' or '%s' in declaration statement", 
					TokenType.COLON_ASSIGN, TokenType.ASSIGN, TokenType.SEMICOLON);
			return null;
		}
		
		bufferer.advance(); //Eat the := or =
		Expression exp = ExpressionParser.parseExpression(bufferer);
		if(exp == null)
			return null; //The expression parser prints errors
		
		return new NameTypeExpr(identifier, type, exp);
	}
}
