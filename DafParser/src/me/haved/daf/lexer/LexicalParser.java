/*package me.haved.daf.lexer;

import java.util.ArrayList;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.text.TextSupplier;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	public static LiveTokenizer tokenizeFile(RegisteredFile file, MacroMap map) {
		try {
			TextSupplier supplier = new PreProcessor(file, map);
			TextBufferer bufferer = new TextBufferer(supplier);
			return new LiveTokenizer(bufferer);
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
				continue; //More whitespaces?
			}
			
			for(Picker picker:pickers) {
				Token token = picker.makeToken(bufferer);
				if(token != null) {
					tokens.add(token);
					continue mainLoop;
				} else 
					bufferer.restoreToStart();
			}
			
			log(bufferer.getFile(), bufferer.getCurrentCol(), bufferer.getCurrentLine(), ERROR, 
					"The char '%c' was totally unknown to the lexical parser!", bufferer.getCurrentChar());
			
			logAssert(bufferer.advance()); //Should always work!
			bufferer.setNewStart(0);
		}
	}
}*/
