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
		}
		else if(isWhitespace(c)) {
			return DONE_PARSING;
		}
		else if(!numbersFound) { //Trying to parse previous text as word token
			logAssert(word.length()>0, "NumberTokenParsrer.parse() called before tryStartParsing()");
			TokenFileLocation location = getTokenFileLocation();
			if(WordTokenParser.instance.tryStartParsing(word.charAt(0), location.fileName, location.lineNumber, location.columnNumber)) {
				String text = word.toString()+c;
				for(int i = 1; i < word.length(); i++) {
					if(WordTokenParser.instance.parse(word.charAt(i), location.lineNumber, location.columnNumber+i) == CONTINUE_PARSING)
						if(i==text.length()-1)
							return NEW_PARSER;
					else
						break;
				}
			}
		}
		return DONE_PARSING;
	}

	@Override
	public Token getReturnedToken() {
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
