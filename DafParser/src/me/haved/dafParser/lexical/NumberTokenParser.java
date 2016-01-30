package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class NumberTokenParser extends TokenParser {

	StringBuilder word = new StringBuilder();
	
	private static final int INTEGER = 0;
	private static final int DOUBLE = 1;
	private static final int FLOAT = 2;
	int parseStyle = INTEGER;
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isNumberSignOrDot(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			parseStyle = c=='.'?DOUBLE:INTEGER;
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
			word.append(c);
			return COUNTINUE_PARSING;
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
		log(fileLocation(getTokenFileLocation().fileName, line, col), ERROR, "Unrecogniced char '%c' when parsing number literal", c);
		return ERROR_PARSING;
	}

	@Override
	public Token getReturnedToken() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getParserName() {
		return "Number literal parser";
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
