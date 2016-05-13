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
			
			while(true) {
				if(TextParserUtil.isAnyWhitespace(bufferer.getCurrentChar())) {
					if(!bufferer.advance()) //Skip the whitespace
						break;
					bufferer.setNewStart(0); //We have now found a new start, after the whitespace!
				}
				
				for(Picker picker:pickers) {
					Token token = picker.makeToken(bufferer);
					if(token != null) {
						tokens.add(token);
						break;
					} else 
						bufferer.restoreToStart();
				}
			}
			
			supplier.close();
			tokensMap.put(fileId, tokens);
			
			return tokens;
		}
		catch(Exception e) {
			e.printStackTrace();
			log(file, FATAL_ERROR, "Some sort of Exception was thrown in tokenizeFile()");
			return null;
		}
	}
}
