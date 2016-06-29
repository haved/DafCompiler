package me.haved.daf.lexer;

import java.util.HashMap;
import java.util.HashSet;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class TokenPicker {
	
	private static HashSet<String> finsihedTokens = new HashSet<>();
	private static HashMap<String, String> mustBeFollowedBy = new HashMap<>();
	
	static {
		finsihedTokens.add("++");
		finsihedTokens.add("--");
		finsihedTokens.add("+=");
		finsihedTokens.add("-=");
		finsihedTokens.add("*=");
		finsihedTokens.add("/=");
		finsihedTokens.add("%=");
		finsihedTokens.add(":=");
		finsihedTokens.add("==");
		finsihedTokens.add("!=");
		finsihedTokens.add("<<");
		finsihedTokens.add(">>");
		finsihedTokens.add("||");
		finsihedTokens.add("&&");
		finsihedTokens.add("(");
		finsihedTokens.add(")");
		finsihedTokens.add("[");
		finsihedTokens.add("]");
		finsihedTokens.add("{");
		finsihedTokens.add("}");
		finsihedTokens.add(",");
		finsihedTokens.add(".");
		finsihedTokens.add(";");
		finsihedTokens.add("?");
		finsihedTokens.add("@");
		finsihedTokens.add("->");
		
		mustBeFollowedBy.put("=", "=");
		mustBeFollowedBy.put(":", "=");
		mustBeFollowedBy.put("!", "=");
		mustBeFollowedBy.put("+", "+=");
		mustBeFollowedBy.put("-", "-=");
		mustBeFollowedBy.put("*", "=");
		mustBeFollowedBy.put("/", "=");
		mustBeFollowedBy.put("<", "<=");
		mustBeFollowedBy.put(">", ">=");
		mustBeFollowedBy.put("<<", "=");
		mustBeFollowedBy.put(">>", "=");
	}
	
	public static Token makeToken(TextBufferer bufferer) {
		RegisteredFile file = bufferer.getFile();
		int line = bufferer.getCurrentLine();
		int col = bufferer.getCurrentCol();
		char firstLetter = bufferer.getCurrentChar();
		
		//This should happen after numbers and strings!
		
		boolean specialChar;
		
		if(TextParserUtil.isStartOfIdentifier(firstLetter)) {
			specialChar = false;
		} else if(TextParserUtil.isLegalTokenSpecialCharacter(firstLetter)) {
			specialChar = true;
		} else {
			//log(fileName, line, col, ERROR, "Illegal char '%c' makes lexer flip out!", firstLetter);
			return null; //Returns back to the other thing
		}
		
		StringBuilder text = new StringBuilder().append(firstLetter);
		
		String name;
		
		while(true) {
			bufferer.advance();
			char letter = bufferer.getCurrentChar();			
			if(specialChar ? !TextParserUtil.isLegalTokenSpecialCharacter(letter) : !TextParserUtil.isIdentifierChar(letter))
				break;
			
			name = text.toString();
			if(finsihedTokens.contains(name))
				break;
			else {
				String req = mustBeFollowedBy.get(name);
				if(req!=null) {
					boolean found = false;
					for(int i = 0; i < req.length(); i++) {
						if(req.charAt(i)==letter) {
							found = true;
							break;
						}
					}
					if(!found)
						break; // "<<" has to be followed by '=' for instance, or else we are done with the current token
				}
			}
			
			text.append(bufferer.getCurrentChar());
		}
		
		name = text.toString();
		
		bufferer.setNewStart(0); //We are guaranteed to add some token here no matter what
		for(TokenType type:TokenType.values()) {
			if(!type.isSpecial() && type.getText().equals(name))
				return new Token(type, file, line, col);
		}
		
		if(specialChar) {
			log(file, line, col, WARNING, "Found special chars with no meaning: '%s'", name);
			return null;
		}
		
		return new Token(TokenType.IDENTIFER, file, line, col, name);
	}
}
