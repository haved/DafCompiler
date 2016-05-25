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
		if(c=='/'){} //Try doing comments and stuff
		else if(c=='#')
			return doFlowMacrosAndArithmatic(c, line, col);//Try doing macros, arithmetic and evaluation
		return forceSetCurrentChar(c, line, col);
	}
	
	private boolean forceSetCurrentChar(char c, int line, int col) {
		this.outputChar = c;
		this.outputLine = line;
		this.outputCol = col;
		return true;
	}
	
	/**
	 * 
	 * @param c the pound symbol causing this to happen
	 * @param line the line of the symbol
	 * @param col the column of the symbol
	 * @return true if the output char was changed to something new
	 */
	private boolean doFlowMacrosAndArithmatic(char c, int line, int col) {
		String token = pickUpPreProcToken();
		
		return false;
	}
	
	private String pickUpPreProcToken() {
		StringBuilder builder = new StringBuilder();
		
		while(true) {
			advanceInput();
			if(!TextParserUtil.isLegalCompilerTokenChar(inputChar))
				break;
			builder.append(inputChar);
		}
		
		String token = builder.toString();
		
		if(TextParserUtil.isStartOfMacroParameters(inputChar)) {
			char startChar = inputChar;
			int  startLine = inputLine;
			int  startCol  = inputCol;
			advanceInput();
			if(!TextParserUtil.isEndOfMacroParameters(inputChar)) { //Oh shit we need to push the stuff we skipped back! Macro parameters, y'know
				pushBufferedChar(inputChar, inputLine, inputCol);
				pushBufferedChar(startChar, startLine, startCol);
			} else {
				return token; //We have skipped the <> so the next char asked for will be the one after
			}
		}
		else if(!TextParserUtil.isAnyWhitespace(inputChar)) {
			log(getFile(), inputLine, inputCol, ERROR, "Special char (%c) found right after compiler token! (%s)", inputChar, token);
		}
		pushBufferedChar(inputChar, inputLine, inputCol); //Put it back!
		
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
