package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class CompilerTokenParser extends TokenParser {

	public static final CompilerTokenParser instance = new CompilerTokenParser();
	
	private StringBuilder word = new StringBuilder();
	
	private static final String INLINE_PARSE_END = "##end";
	
	private static final int WORD_SEARCH_PARSE = 0;
	private static final int INLINE_PARSE = 1;
	private static final int FILENAME_PARSE = 2;
	
	private static final int NO_PARSE_SUB_TYPE = 0;
	private static final int INLINE_TYPE_CPP = 1;
	private static final int INLINE_TYPE_HEADER = 2;
	private static final int FILE_TYPE_IMPORT = 1;
	private static final int FILE_TYPE_USING = 2;
	
	//Happens later anyways
	private int parseStyle = WORD_SEARCH_PARSE;
	private int parseSubType = NO_PARSE_SUB_TYPE;
	private int inlineParseEnd = 0; //How much of the ##end keyword has been parsed
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isCharCompilerPound(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			parseStyle = WORD_SEARCH_PARSE;
			parseSubType = NO_PARSE_SUB_TYPE;
			inlineParseEnd = 0;
			return true;
		}
		return false;
	}
	
	@Override
	public int parse(char c, int line, int col) {
		if(parseStyle == WORD_SEARCH_PARSE) {
			if(TokenParser.isWhitespace(c)) { //The type of compiler message is decided
				String keyword = word.toString();
				if(keyword.equals(TokenType.DAF_CPP.getKeyword())) {
					log(getTokenFileLocation().getErrorString(), TOKEN_DEBUG, "Started inline cpp.");
					word.setLength(0);
					parseStyle = INLINE_PARSE;
					parseSubType = INLINE_TYPE_CPP;
				}
				else if(keyword.equals(TokenType.DAF_HEADER.getKeyword())) {
					log(getTokenFileLocation().getErrorString(), TOKEN_DEBUG, "Started inline header code.");
					word.setLength(0);
					parseStyle = INLINE_PARSE;
					parseSubType = INLINE_TYPE_HEADER;
				}
				else if(keyword.equals(TokenType.DAF_IMPORT.getKeyword())) {
					word.setLength(0);
					parseStyle = FILENAME_PARSE;
					parseSubType = FILE_TYPE_IMPORT;
				}
				else if(keyword.equals(TokenType.DAF_USING.getKeyword())) {
					word.setLength(0);
					parseStyle = FILENAME_PARSE;
					parseSubType = FILE_TYPE_USING;
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
		} else if(parseStyle == FILENAME_PARSE) {
			if(isNewline(c))
				return DONE_PARSING;
			word.append(c);
			return CONTINUE_PARSING;
		}
		return ERROR_PARSING; //Why are you here??
	}

	@Override
	public Token getReturnedToken() {
		if(parseStyle == INLINE_PARSE) {
			String inline = word.toString().substring(0, word.length()-INLINE_PARSE_END.length());
			return new Token(parseSubType == INLINE_TYPE_CPP ? TokenType.DAF_CPP : TokenType.DAF_HEADER, getTokenFileLocation(), inline);
		} else if(parseStyle == FILENAME_PARSE) {
			String file = word.toString().trim();
			int firstH = file.indexOf('\"');
			if(firstH < 0) {
				log(getTokenFileLocation().getErrorString(), ERROR, "No \"quote\" symbol was found in import/using statement!");
				return null;
			}
			if(firstH > 0) {
				log(getTokenFileLocation().getErrorString(), WARNING, "Junk found before file in import/using statement");
			}
			int nextH = file.indexOf('\"', firstH+1);
			if(nextH < 0) {
				log(getTokenFileLocation().getErrorString(), ERROR, "Only one \"quote\" symbol was found in import/using statement!");
				return null;
			}
			if(nextH<file.length()-1) {
				log(getTokenFileLocation().getErrorString(), WARNING, "Junk found after import/using statement");
			}
			
			String name = file.substring(firstH+1, nextH);
			return new Token(parseSubType == FILE_TYPE_IMPORT ? TokenType.DAF_IMPORT : TokenType.DAF_USING, getTokenFileLocation(), name);
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
