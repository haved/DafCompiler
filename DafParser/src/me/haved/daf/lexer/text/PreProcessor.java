package me.haved.daf.lexer.text;

import java.util.Stack;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.depricated.MacroMap;
import me.haved.daf.lexer.text.directives.DirectiveHandler;
import me.haved.daf.lexer.text.directives.MacroDirectiveHandler;

import static me.haved.daf.LogHelper.*;

public class PreProcessor implements TextSupplier {
	
	private static DirectiveHandler[] DIRECTIVE_HANDLERS = {MacroDirectiveHandler::handleDirective};
	
	private InputHandler inputHandler;
	
	private char outputChar;
	private int  outputLine;
	private int  outputCol;
	
	public PreProcessor(RegisteredFile file, MacroMap macros) {
		inputHandler = new InputHandler(file, macros);
		
		trySetCurrentChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol());
	}
	
	@Override
	public boolean advance() {
		while(true) {
			if(!inputHandler.advanceInput())
				return false;
			
			//Keep going until we actually set the output char!
			if(trySetCurrentChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol()))
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
		inputHandler.advanceInput();
		if(inputHandler.getInputChar() == '/') {
			while(true) {
				if(!inputHandler.advanceInput())
					return false;
				if(inputHandler.getInputChar()=='\n')
					break;
			}
			return false; //We then pick up the char after the newline
		} else if(inputHandler.getInputChar() == '*') {
			boolean prevStar = true;
			while(true) {
				if(!inputHandler.advanceInput())
					return false;
				if(prevStar && inputHandler.getInputChar() == '/')
					break;
				prevStar = inputHandler.getInputChar() == '*';
			}
			return false; //We then pick up the char after the */
		}
		else {
			inputHandler.pushBufferedChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol());
			forceSetCurrentChar(c, line, col);
			return true; //We set it ourself
		}
	}
	
	/** 
	 * @param c the pound symbol causing this to happen
	 * @param line the line of the pound symbol
	 * @param col the column of the pound symbol
	 * @return true if the output char was changed to something new
	 */
	private boolean doFlowMacrosAndArithmetic(char c, int line, int col) {
		inputHandler.advanceInput();
		if(inputHandler.getInputChar() == '#')
			return forceSetCurrentChar(inputHandler.getInputChar(), line, col);
		
		String directive = pickUpPreProcDirective(); //After, the char immediately after the directive is on the stack
		
		for(DirectiveHandler handler:DIRECTIVE_HANDLERS) {
			int result = handler.handleDirective(directive, line, col, inputHandler);
			if(result != DirectiveHandler.CANT_HANLDE_DIRECTIVE)
				return false;
		}
		
		log(getFile(), line, col, ERROR, "Found directive ' #%s ' that the preprocessor can't handle!", directive);
		
		return false;
	}
	
	private String pickUpPreProcDirective() {
		StringBuilder builder = new StringBuilder();
		
		while(true) {
			if(!TextParserUtil.isLegalDirectiveChar(inputHandler.getInputChar()))
				break;
			builder.append(inputHandler.getInputChar());
			
			if(TextParserUtil.isOneLetterCompilerToken(inputHandler.getInputChar())) {
				inputHandler.advanceInput();
				break;
			}
			inputHandler.advanceInput();
		}
		
		//InputChar is now the 1. outside the compiler token
		
		String token = builder.toString();
		
		if(TextParserUtil.isStartOfMacroParameters(inputHandler.getInputChar())) {
			char startChar = inputHandler.getInputChar();
			int  startLine = inputHandler.getInputLine();
			int  startCol  = inputHandler.getInputCol ();
			inputHandler.advanceInput();
			
			if(!TextParserUtil.isEndOfMacroParameters(inputHandler.getInputChar())) {
				//Oh shit we need to push the stuff we skipped back! Macro parameters, y'know
				inputHandler.pushBufferedChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol());
				inputHandler.pushBufferedChar(startChar, startLine, startCol);
			}
			
			return token; //We have skipped the <>, or added what comes next back, so we're good to go
		}
		
		//Put whatever came after the token back!
		inputHandler.pushBufferedChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol());
		
		return token;
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
		return inputHandler.getFile();
	}

	@Override
	public void close() {
		inputHandler.close();
	}
	
	/** Class used by directive handlers to access the preprocessors file input and stuff
	 * TODO: Make the class the input supplier. Handling bufferers and the file text supplier
	 * @author havard
	 */
	public class InputHandler {
		
		private TextSupplier fileInput;
		private Stack<MacroMap> macros;

		private Stack<Character> bufferedInputChars;
		private Stack<Integer>   bufferedInputLines;
		private Stack<Integer>   bufferedInputColms;
		
		private char inputChar;
		private int  inputLine;
		private int  inputCol;
		
		private InputHandler(RegisteredFile file, MacroMap macros) {
			this.fileInput = new FileTextSupplier(file);
			
			this.macros = new Stack<>();
			this.macros.push(macros);
			
			bufferedInputChars = new Stack<>();
			bufferedInputLines = new Stack<>();
			bufferedInputColms = new Stack<>();
			
			inputChar = fileInput.getCurrentChar();
			inputLine = fileInput.getCurrentLine();
			inputCol  = fileInput.getCurrentCol();
		}
		
		public boolean advanceInput() {
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
		
		private void pushBufferedChar(char c, int line, int col) {
			bufferedInputChars.push(c);
			bufferedInputLines.push(line);
			bufferedInputColms.push(col);
		}
		
		public char getInputChar() {
			return inputChar;
		}
		public int getInputLine() {
			return inputLine;
		}
		public int getInputCol() {
			return inputCol;
		}
		public RegisteredFile getFile() {
			return fileInput.getFile();
		}
		public Stack<MacroMap> getMacroStack() {
			return macros;
		}
		public void close() {
			fileInput.close();
		}
	}
}
