package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class CompilerTokenParser extends TokenParser {

	public static final CompilerTokenParser instance = new CompilerTokenParser();
	
	private StringBuilder word = new StringBuilder();
	
	private static final String INLINE_PARSE_END = "##end";
	
	private static final int WORD_SEARCH_PARSE = 0;
	private static final int INLINE_PARSE = 1;
	private static final int INLINE_TYPE_CPP = 0;
	private static final int INLINE_TYPE_HEADER = 1;
	
	private int parseStyle = WORD_SEARCH_PARSE;
	private int inlineParseType = -1;
	private int inlineParseEnd = 0;
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isCharCompilerPound(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			parseStyle = WORD_SEARCH_PARSE;
			inlineParseType = -1;
			inlineParseEnd = 0;
			return true;
		}
		return false;
	}
	
	@Override
	public int parse(char c, int line, int col) {
		if(parseStyle == WORD_SEARCH_PARSE) {
			if(word.length()==1) {
				if(!isCharCompilerPound(c)) {
					log(getTokenFileLocation().getErrorString(), ERROR, "Only one pound symbol found. Daf uses double!");
					return ERROR_PARSING;
				}
				word.append(c);
				return CONTINUE_PARSING;
			}
			
			if(TokenParser.isWhitespace(c)) { //The type of compiler message is decided
				String keyword = word.toString();
				if(keyword.equals(TokenType.DAF_CPP.getKeyword())) {
					log(getTokenFileLocation().getErrorString(), MESSAGE, "Started inline cpp.");
					word.setLength(0);
					parseStyle = INLINE_PARSE;
					inlineParseType = INLINE_TYPE_CPP;
				}
				else if(keyword.equals(TokenType.DAF_HEADER.getKeyword())) {
					log(getTokenFileLocation().getErrorString(), MESSAGE, "Started inline header code.");
					word.setLength(0);
					parseStyle = INLINE_PARSE;
					inlineParseType = INLINE_TYPE_HEADER;
				}
				else {
					log(getTokenFileLocation().getErrorString(), ERROR, "'%s' Not a valid compiler message at this point.", keyword);
					return ERROR_PARSING;
				}
			}
			word.append(c);
			return CONTINUE_PARSING;
		} else if(parseStyle == INLINE_PARSE) {
			if(inlineParseEnd >= INLINE_PARSE_END.length())
				return DONE_PARSING;
			word.append(c);
			if(INLINE_PARSE_END.charAt(inlineParseEnd) == c) {
				inlineParseEnd++;
			} else
				inlineParseEnd = 0;
			return CONTINUE_PARSING; //If we quit as soon as the 'd' in '##end' is met, it is parsed as an identifier.
		}
		return ERROR_PARSING; //Why are you here??
	}

	@Override
	public Token getReturnedToken() {
		if(parseStyle == INLINE_PARSE) {
			String inline = word.toString().substring(0, word.length()-INLINE_PARSE_END.length());
			return new Token(inlineParseType == INLINE_TYPE_CPP ? TokenType.DAF_CPP : TokenType.DAF_HEADER, getTokenFileLocation(), inline);
		}
		log(getTokenFileLocation().getErrorString(), ERROR, "Tried to get a token from unfinished CompilerTokenParser!");
		return null;
	}
	
	public static boolean isCharCompilerPound(char c) {
		return c == '#';
	}

	@Override
	public String getParserName() {
		return "Compiler Token Parser";
	}
}
