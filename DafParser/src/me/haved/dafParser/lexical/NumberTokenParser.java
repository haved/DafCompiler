package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class NumberTokenParser extends TokenParser {

	public static final NumberTokenParser instance = new NumberTokenParser();
	
	private StringBuilder word = new StringBuilder();
	
	private static final int INTEGER = 0;
	private static final int DOUBLE = 1;
	private static final int FLOAT = 2;
	private static final TokenType[] TOKEN_TYPES = new TokenType[]{TokenType.INTEGER_LITERAL, TokenType.DOUBLE_LITERAL, TokenType.FLOAT_LITERAL};
	int parseStyle = INTEGER;
	boolean numbersFound=false;
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isNumberSignOrDot(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			parseStyle = c=='.'?DOUBLE:INTEGER;
			numbersFound = isNumber(c);
			return true;
		}
		return false;
	}

	@Override
	public int parse(char c, int line, int col) {
		if(isNumber(c)) {
			if(parseStyle==FLOAT) {
				log(fileLocation(getTokenFileLocation().fileName, line, col), ERROR, "A the number %c was found after 'f'", c);
				return ERROR_PARSING;
			}
			numbersFound = true;
			word.append(c);
			return CONTINUE_PARSING;
		}
		else if(isDot(c)) {
			if(parseStyle==FLOAT) {
				log(fileLocation(getTokenFileLocation().fileName, line, col), ERROR, "A dot was found after 'f'");
				return ERROR_PARSING;
			} else if(parseStyle==DOUBLE) {
				log(fileLocation(getTokenFileLocation().fileName, line, col), ERROR, "A dot was found in the decimal places!");
				return ERROR_PARSING;
			}
			word.append(c);
			parseStyle = DOUBLE;
			return CONTINUE_PARSING;
		}
		else if(isF(c)) {
			if(parseStyle==FLOAT) {
				log(fileLocation(getTokenFileLocation().fileName, line, col), ERROR, "Another 'f' was found after 'f'");
				return ERROR_PARSING;
			}
			if(parseStyle!=DOUBLE) {
				log(fileLocation(getTokenFileLocation().fileName, line, col), ERROR, "An 'f' was found in an integer literal!");
				return ERROR_PARSING;
			}
			word.append(c);
			parseStyle = FLOAT;
			return CONTINUE_PARSING;
		}
		else if(isWhitespace(c) && numbersFound) {
			return DONE_PARSING;
		}
		else if(!numbersFound) {
			logAssert(word.length()>0, "NumberTokenParsrer.parse() called before tryStartParsing()");
			word.append(c);
			return NEW_PARSER;
		}
		return DONE_PARSING;
	}

	@Override
	public Token getReturnedToken() {
		logAssert(numbersFound, "NumberTokenParser.getReturnedToken() called with no numbers!");
		return new Token(TOKEN_TYPES[parseStyle], getTokenFileLocation(), word.toString());
	}

	@Override
	public String getParserName() {
		return "Number literal parser";
	}
	
	@Override
	public TokenParser getWantedTokenParser() {
		return WordTokenParser.instance;
	}
	
	@Override
	public String getNewParserWord() {
		return word.toString();
	}
	
	public static boolean isNumberSignOrDot(char c) {
		return (c>='0' && c<='9') | c == '.' | c == '-' | c == '+';
	}
	
	public static boolean isDot(char c) {
		return c == '.';
	}
	
	public static boolean isF(char c) {
		return c == 'f';
	}
}
