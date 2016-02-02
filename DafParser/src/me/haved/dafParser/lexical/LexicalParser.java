package me.haved.dafParser.lexical;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import static me.haved.dafParser.LogHelper.*;

public class LexicalParser {
	private static TokenParser[] defaultParsers = new TokenParser[] {NumberTokenParser.instance, TextTokenParser.instance, WordTokenParser.instance, CompilerTokenParser.instance};
	
	private File inputFile;
	private String infileName;
	private ArrayList<Token> tokens;
	
	public LexicalParser(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		tokens = new ArrayList<>();
	}
	
	public ArrayList<Token> getTokens() {
		return tokens;
	}
	
	public void parse() {
		logAssert(inputFile.isFile(), "File given to LexicalParser doesn't exist");
		
		try {
			try(BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				parseFromReader(reader);
			}
			catch(Exception e) {
				throw e;
			}
		}
		catch(Exception e) {
			e.printStackTrace(out);
			log(FATAL_ERROR, "LexicalParser.parse(): exception thrown");
		}
	}
	
	private void parseFromReader(BufferedReader reader) throws Exception {
		TokenParser parser=null;
		String previousChars = null;
		int previousCharsRead = 0;
		
		int input;
		char character;
		int line = 1;
		int col = 0;
		
		while(true) {
			col++;
			if(previousChars!=null && previousCharsRead < previousChars.length()) {
				character = previousChars.charAt(previousCharsRead);
				input = character;
				previousCharsRead++;
			} else {
				previousChars = null;
				input = reader.read();
				character = input==-1?'\n':(char) input;
			}
			
			if(parser!=null) {
				int status = parser.parse(character, line, col);
				if(status==TokenParser.NEW_PARSER) {
					previousChars = parser.getNewParserWord();
					TokenFileLocation prevLoc = parser.getNewParserTokenFileLocation();
					line = prevLoc.lineNumber;
					col = prevLoc.columnNumber;
					parser = parser.getWantedTokenParser();
					logAssert(parser.tryStartParsing(previousChars.charAt(0), infileName, line, col), "New parser didn't start");
					previousCharsRead = 1;
				}
				else if(status==TokenParser.DONE_PARSING) {
					Token token = parser.getReturnedToken();
					if(token != null) {
						//log(token.getLocation().getErrorString(), TOKEN_DEBUG, "Token of type '%s' added: %s", token.getType().name(), token.getText());
						tokens.add(token);
					} else
						log(fileLocation(infileName, line, col), WARNING, "No token returned from %s!", parser.getParserName());
					parser = null;
				}
				else if(status == TokenParser.ERROR_PARSING) {
					parser = null;
				}
			}
			if(parser==null & !TokenParser.isWhitespace(character)) {
				for(int i = 0; i < defaultParsers.length; i++) {
					if(defaultParsers[i].tryStartParsing(character, infileName, line, col)) {
						parser = defaultParsers[i];
						break;
					}
				}
				if(parser==null)
					log(fileLocation(infileName, line, col), ERROR, "Couldn't parse character: '%c'", character);
			}
			
			if(input==-1) {
				if(parser!=null)
					log(fileLocation(infileName, line, col), ERROR, "Parsing of %s was interruped by sudden end of file", parser.getParserName());
				break;
			}
			if(TokenParser.isNewline(character)) {
				col = 0;
				line++;
			}
		}
		terminateIfErrorsLogged();
		log(fileLocation(infileName, line, col), MESSAGE, "Finished Lexical Parsing with %d tokens!", tokens.size());
	}
}
