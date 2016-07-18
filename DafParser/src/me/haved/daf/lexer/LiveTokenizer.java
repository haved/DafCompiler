package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.TokenBufferer;

import static me.haved.daf.LogHelper.*;

public class LiveTokenizer implements TokenBufferer {

	private static Picker[] pickers = {NumberLiteralPicker::makeToken, StringLiteralPicker::makeToken, TokenPicker::makeToken};//Fancy java
	
	private TextBufferer bufferer;
	
	private Token current;
	private Token lookahead;
	private boolean done;
	
	public LiveTokenizer(RegisteredFile file, MacroMap macros) {
		this.bufferer = new TextBufferer(new PreProcessor(file, macros));
		current = getToken();
		lookahead = getToken();
		
		if(current == null)
			done = true;
	}
	
	@Override
	public Token getCurrentToken() {
		logAssert(!done);
		return current;
	}
	
	@Override
	public Token getLookaheadToken() {
		return lookahead;
	}

	@Override
	public Token getLastToken() {
		logAssert(done);
		return current;
	}
	
	@Override
	public Token getLastOrCurrent() {
		return done ? getLastToken() : getCurrentToken();
	}

	@Override
	public boolean hasLookaheadToken() {
		return lookahead != null;
	}

	@Override
	public boolean hasCurrentToken() {
		return !done;
	}

	@Override
	public boolean advance() {
		if(lookahead == null || done) {
			done = true;
			return false;
		}
		
		current = lookahead;
		lookahead = getToken();
		return true;
	}
	
	private Token getToken() {
		while(true) { //Either go until the file is over, or a token is found!
			if(TextParserUtil.isAnyWhitespace(bufferer.getCurrentChar())) {
				if(!bufferer.advance())
					return null;
				bufferer.setNewStart(0);
				continue; //Check for more whitespaces!
			}
				
			for(Picker picker:pickers) {
				Token token = picker.makeToken(bufferer);
				if(token != null) {
					return token;
				} else 
					bufferer.restoreToStart();
			}
				
			log(bufferer.getFile(), bufferer.getCurrentCol(), bufferer.getCurrentLine(), ERROR, 
					"The char '%c' was totally unknown to the lexical parser!", bufferer.getCurrentChar());
			
			logAssert(bufferer.advance()); //Should always work, because the file must end with a newline
			bufferer.setNewStart(0);
		}
	}

	@Override
	public boolean isCurrentTokenOfType(TokenType type) {
		return current == null ? false : current.getType() == type;
	}
}
