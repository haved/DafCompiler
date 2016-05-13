package me.haved.daf.lexer;

import java.util.ArrayList;
import java.util.HashMap;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.CodeSupplier;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	
	private static HashMap<Integer, ArrayList<Token>> tokensMap = new HashMap<>();
	
	private static Picker[] pickers = {TokenPicker::makeToken};//Fancy java
	
	public static ArrayList<Token> tokenizeFile(RegisteredFile file, MacroMap map) {
		int fileId = file.getId();
		try {
			log(DEBUG, "tokenizeFile(%s) with file id %d", file.toString(), fileId);
			
			if(tokensMap.containsKey(fileId))
				return tokensMap.get(fileId);
			
			ArrayList<Token> tokens = new ArrayList<>();
			CodeSupplier supplier = new CodeSupplier(file, map);
			TextBufferer bufferer = new TextBufferer(supplier);
			
			fillTokenList(tokens, bufferer);
			
			supplier.close();
			tokensMap.put(fileId, tokens);
			
			for(int i = 0; i < tokens.size(); i++) {
				System.out.println(tokens.get(i).getErrorString());
			}
			
			return tokens;
		}
		catch(Exception e) {
			e.printStackTrace();
			log(file, FATAL_ERROR, "Some sort of Exception was thrown in tokenizeFile()");
			return null;
		}
	}
	
	private static void fillTokenList(ArrayList<Token> tokens, TextBufferer bufferer) {
		mainLoop:
		while(true) {
			if(TextParserUtil.isAnyWhitespace(bufferer.getCurrentChar())) {
				if(!bufferer.advance()) //Skip the whitespace
					break;
				bufferer.setNewStart(0); //We have now found a new start, after the whitespace!
				continue;
			}
			
			for(Picker picker:pickers) {
				Token token = picker.makeToken(bufferer);
				if(token != null) {
					tokens.add(token);
					continue mainLoop;
				} else 
					bufferer.restoreToStart();
			}
			
			log(bufferer.getSourceName(), bufferer.getCurrentCol(), bufferer.getCurrentLine(), ERROR, 
					"The char '%c' was totally unknown to the lexical parser!", bufferer.getCurrentChar());
			
			logAssert(bufferer.advance()); //Should always work!
			bufferer.setNewStart(0);
		}
	}
}
