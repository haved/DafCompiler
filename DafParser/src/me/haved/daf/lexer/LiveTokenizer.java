package me.haved.daf.lexer;

import java.util.ArrayList;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.TokenBufferer;

public class LiveTokenizer implements TokenBufferer {

	private static Picker[] pickers = {NumberLiteralPicker::makeToken, StringLiteralPicker::makeToken, TokenPicker::makeToken};//Fancy java
	
	private TextBufferer bufferer;
	
	private ArrayList<Token> tokenBuffer;
	
	public LiveTokenizer(RegisteredFile file, MacroMap macros) {
		this.bufferer = new TextBufferer(new PreProcessor(file, macros));
		this.tokenBuffer = new ArrayList<>();
	}
	
	@Override
	public Token getCurrentToken() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Token getLastToken() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean hasCurrentToken() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean advance() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isCurrentTokenOfType(TokenType type) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void resetToBase() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void updateBase(int offset) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void forgetBase() {
		// TODO Auto-generated method stub
		
	}

}
