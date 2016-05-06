package me.haved.daf.lexer;

import java.util.ArrayList;
import java.util.HashMap;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	
	private static HashMap<Integer, ArrayList<Token>> tokensMap = new HashMap<>();
	
	public static ArrayList<Token> tokenizeFile(RegisteredFile file, MacroMap map) {
		int fileId = file.getId();
		try {
			log(DEBUG, "tokenizeFile(%s) with file id %d", file.toString(), fileId);
			
			if(tokensMap.containsKey(fileId))
				return tokensMap.get(fileId);
			
			ArrayList<Token> tokens = new ArrayList<>();
			CodeSupplier supplier = new CodeSupplier(file, map);
			
			
			
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
