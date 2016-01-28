package me.haved.dafParser.lexical;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import static me.haved.dafParser.LogHelper.*;

public class LexicalParser {
	private static TokenParser[] defaultParsers = new TokenParser[] {WordTokenParser.instance, CompilerTokenParser.instance};
	
	private File inputFile;
	private String infileName;
	private ArrayList<Token> tokens;
	
	public LexicalParser(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		tokens = new ArrayList<>();
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
		log(INFO, "Finished Lexical Parsing...");
	}
	
	private void parseFromReader(BufferedReader reader) throws Exception {
		TokenParser parser=null;
		int input;
		char character;
		int line = 1;
		int col = 0;
		
		while(true) {
			col++;
			input = reader.read();
			character = input==-1?'\n':(char) input;
			
			if(parser!=null) {
				int status = parser.parse(character, line, col);
				if(status==0) {
					tokens.add(parser.getReturnedToken());
					Token token = tokens.get(tokens.size()-1);
					log(token.getLocation().getErrorString(), MESSAGE, "Token of type '%s' added: %s", token.getType().name(), token.getText());
					parser = null;
				}
				else if(status == -1) {
					parser = null;
				}
			}
			if(parser==null) {
				for(int i = 0; i < defaultParsers.length; i++) {
					if(defaultParsers[i].tryStartParsing(character, infileName, line, col)) {
						parser = defaultParsers[i];
					}
				}
			}
			
			if(input==-1)
				break;
			if(TokenParser.isNewline(character)) {
				col = 0;
				line++;
			}
		}
		terminateIfErrorsLogged();
		log(fileLocation(infileName, line, col), MESSAGE, "Finished Lexical Parsing with %d tokens!", tokens.size());
	}
}
