package me.haved.daf.lexer.text;

import java.util.Stack;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.depricated.MacroMap;

import static me.haved.daf.LogHelper.*;

public class PreProcessor implements TextSupplier {
	
	private TextSupplier fileInput;
	private Stack<MacroMap> macros;

	private Stack<Character> bufferedInputChars;
	private Stack<Integer>    bufferedInputLines;
	private Stack<Integer>    bufferedInputColms;
	
	private char inputChar;
	private int  inputLine;
	private int  inputCol;
	
	private char outputChar;
	private int  outputLine;
	private int  outputCol;
	
	public PreProcessor(RegisteredFile file, MacroMap macros) {
		this.fileInput = new FileTextSupplier(file);
		
		this.macros = new Stack<>();
		this.macros.push(macros);
		
		bufferedInputChars = new Stack<>();
		bufferedInputLines = new Stack<>();
		bufferedInputColms = new Stack<>();
		
		trySetCurrentChar(fileInput.getCurrentChar(), fileInput.getCurrentLine(), fileInput.getCurrentCol());
	}
	
	private boolean advanceInput() {
		if(!bufferedInputChars.isEmpty()) {
			inputChar = bufferedInputChars.pop();
			inputLine = bufferedInputLines.pop();
			inputCol  = bufferedInputColms.pop();
			return true;
		}
		if(!fileInput.advance())
			return false;
		
		inputChar = fileInput.getCurrentChar();
		inputLine = fileInput.getCurrentLine();
		inputCol  = fileInput.getCurrentCol();
		return true;
	}
	
	@Override
	public boolean advance() {
		while(true) {
			if(!advanceInput())
				return false;
			
			if(trySetCurrentChar(inputChar, inputLine, inputCol)) //Keep going until we actually set the output char!
				break;
		}
		return true;
	}
	
	private boolean trySetCurrentChar(char c, int line, int col) {
		if(c=='/')
			return doCommentChecks(c, line, col);//Try doing comments and stuff
		else if(c=='#')
			return doFlowMacrosAndArithmetic(c, line, col);//Try doing macros, arithmetic and evaluation
		return forceSetCurrentChar(c, line, col);
	}
	
	private boolean forceSetCurrentChar(char c, int line, int col) {
		this.outputChar = c;
		this.outputLine = line;
		this.outputCol = col;
		return true;
	}
	
	private boolean doCommentChecks(char c, int line, int col) {
		advanceInput();
		if(inputChar == '/') {
			while(true) {
				if(!advanceInput())
					return false;
				if(inputChar=='\n')
					break;
			}
			return false; //We then pick up the char after the newline
		} else if(inputChar == '*') {
			boolean prevStar = true;
			while(true) {
				if(!advanceInput())
					return false;
				if(prevStar && inputChar == '/')
					break;
				prevStar = inputChar == '*';
			}
			return false; //We then pick up the char after the */
		}
		else {
			pushBufferedChar(inputChar, inputLine, inputCol);
			forceSetCurrentChar(c, line, col);
			return true; //We set it ourself
		}
	}
	
	/**
	 * 
	 * @param c the pound symbol causing this to happen
	 * @param line the line of the symbol
	 * @param col the column of the symbol
	 * @return true if the output char was changed to something new
	 */
	private boolean doFlowMacrosAndArithmetic(char c, int line, int col) {
		advanceInput();
		if(inputChar == '#')
			return forceSetCurrentChar(inputChar, line, col);
		
		String token = pickUpPreProcToken(); //After, The next char from advanceInput() is ready, so return false;
		
		log(DEBUG, "Found compiler token: %s", token);
		
		return false;
	}
	
	private String pickUpPreProcToken() {
		StringBuilder builder = new StringBuilder();
		
		while(true) {
			if(!TextParserUtil.isLegalCompilerTokenChar(inputChar))
				break;
			builder.append(inputChar);
			
			if(TextParserUtil.isOneLetterCompilerToken(inputChar)) {
				advanceInput();
				break;
			}
			advanceInput();
		}
		
		//InputChar is now the 1. outside the compiler token
		
		String token = builder.toString();
		
		if(TextParserUtil.isStartOfMacroParameters(inputChar)) {
			char startChar = inputChar;
			int  startLine = inputLine;
			int  startCol  = inputCol;
			advanceInput();
			if(!TextParserUtil.isEndOfMacroParameters(inputChar)) { //Oh shit we need to push the stuff we skipped back! Macro parameters, y'know
				pushBufferedChar(inputChar, inputLine, inputCol);
				pushBufferedChar(startChar, startLine, startCol);
			}
			
			return token; //We have skipped the <>, or added what comes next back, so we're good to go
		}
		
		pushBufferedChar(inputChar, inputLine, inputCol); //Put whatever came after the token back!
		
		return token;
	}
	
	private void pushBufferedChar(char c, int line, int col) {
		bufferedInputChars.push(c);
		bufferedInputLines.push(line);
		bufferedInputColms.push(col);
	}
	
	@Override
	public char getCurrentChar() {
		return outputChar;
	}

	@Override
	public int getCurrentLine() {
		return outputLine;
	}

	@Override
	public int getCurrentCol() {
		return outputCol;
	}

	@Override
	public RegisteredFile getFile() {
		return fileInput.getFile();
	}

	@Override
	public void close() {
		fileInput.close();
	}
}
