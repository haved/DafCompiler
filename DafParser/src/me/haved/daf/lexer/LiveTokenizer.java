package me.haved.daf.lexer;

import java.util.ArrayList;

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
	
	private ArrayList<Token> tokenBuffer;
	private int currentTokenInBuffer;
	
	private Token currentToken;
	private boolean rememberBase;
	private boolean done;
	
	public LiveTokenizer(RegisteredFile file, MacroMap macros) {
		this.bufferer = new TextBufferer(new PreProcessor(file, macros));
		this.tokenBuffer = new ArrayList<>();
		this.rememberBase = true;
		this.currentTokenInBuffer = 0;
		
		Token first = getToken();
		if(first == null) {
			log(file, DEBUG, "Not a single token was to be had from the file!");
			done = true;
		} else
			tokenBuffer.add(first);
	}
	
	@Override
	public Token getCurrentToken() {
		return rememberBase ? tokenBuffer.get(currentTokenInBuffer) : currentToken;
	}

	@Override
	public Token getLastToken() {
		logAssert(done);
		return rememberBase ? tokenBuffer.get(currentTokenInBuffer) : currentToken;
	}

	@Override
	public boolean hasCurrentToken() {
		return !done;
	}

	@Override
	public boolean advance() {
		if(rememberBase && currentTokenInBuffer < tokenBuffer.size()-1) {
			currentTokenInBuffer++;
			return true;
		}
		
		Token next = getToken();
		if(next == null) {
			done = true;
			return false;
		}
		
		if(rememberBase) {
			tokenBuffer.add(next);
			log(next, DEBUG, "TokenBufferSize: %d", tokenBuffer.size());
			currentTokenInBuffer++;
		}
		else
			currentToken = next;
		
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
		return hasCurrentToken() && getCurrentToken().getType() == type;
	}

	@Override
	public void resetToBase() {
		this.currentTokenInBuffer = 0;
	}

	@Override
	public void updateBase(int offset) {
		if(rememberBase) {
			currentToken = tokenBuffer.get(tokenBuffer.size()-1);
			rememberBase = false;
			tokenBuffer.clear();
		}
		logAssert(offset >= 0);
		for(int i = offset; i > 0; i--) {
			advance();
		}
		rememberBase = true;
		tokenBuffer.add(currentToken);
		this.currentTokenInBuffer = 0;
	}

	@Override
	public void forgetBase() {
		rememberBase = false;
		currentToken = tokenBuffer.get(tokenBuffer.size()-1);
		tokenBuffer.clear();
	}

}
