package me.haved.daf.data.statement;

import static me.haved.daf.LogHelper.ERROR;
import static me.haved.daf.LogHelper.log;

import java.util.ArrayList;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.ExpressionParser;
import me.haved.daf.syxer.TokenBufferer;

public class FunctionCall extends NodeBase implements Statement, Expression {
	private String name;
	private Expression[] parameters;
	
	public FunctionCall(String name, Expression[] parameters) {
		this.name = name;
		this.parameters = parameters;
	}
	
	public String getName() {
		return name;
	}
	
	public boolean hasParameters() {
		return parameters != null && parameters.length > 0;
	}
	
	public Expression[] getParameters() {
		return parameters;
	}
	
	@Override
	public String getSignature() {
		StringBuilder out = new StringBuilder(name).append("(");
		if(parameters != null)
			for(int i = 0; i < parameters.length; i++) {
				if(i != 0)
					out.append(", ");
				out.append(parameters[i]);
			}
		
		return out.append(")").toString();
	}
	
	public static FunctionCall parseParameters(String name, TokenBufferer bufferer) {
		if(!bufferer.advance()) //Eat (
				{ log(bufferer.getLastToken(), ERROR, "Expected ')' before EOF"); return null; }
		if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			bufferer.advance(); //Eat )
			return new FunctionCall(name, null);
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
		
		bufferer.advance(); //Eat )
		return new FunctionCall(name, params.toArray(new Expression[params.size()]));
	}
}
