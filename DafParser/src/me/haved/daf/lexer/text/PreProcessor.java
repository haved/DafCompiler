package me.haved.daf.lexer.text;

import java.util.Stack;

import me.haved.daf.RegisteredFile;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.lexer.text.directives.DirectiveHandler;
import me.haved.daf.lexer.text.directives.ExpressionDirectiveHandler;
import me.haved.daf.lexer.text.directives.IfDirectiveHandler;
import me.haved.daf.lexer.text.directives.MacroDirectiveHandler;
import me.haved.daf.lexer.text.directives.MacroEvaluationDirectiveHandler;
import me.haved.daf.lexer.text.directives.PopMacroStackDirectiveHandler;
import me.haved.daf.lexer.text.directives.PreProcessorController;

import static me.haved.daf.LogHelper.*;

public class PreProcessor implements TextSupplier {
	
	private static DirectiveHandler[] DIRECTIVE_HANDLERS = {MacroDirectiveHandler::handleDirective, MacroEvaluationDirectiveHandler::handleDirective,
			PopMacroStackDirectiveHandler::handleDirective, new IfDirectiveHandler(), ExpressionDirectiveHandler::handleDirective};
	
	private InputHandler inputHandler;
	
	private char outputChar;
	private int  outputLine;
	private int  outputCol;
	
	private Stack<PreProcessorController> controllers;
	
	public PreProcessor(RegisteredFile file, MacroMap macros) {
		inputHandler = new InputHandler(file, macros);
		controllers = new Stack<>();
		
		trySetCurrentChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol());
	}
	
	@Override
	public boolean advance() {
		while(true) {
			if(!inputHandler.advanceInput()) {
				if(!controllers.isEmpty())
					log(getFile(), inputHandler.getInputLine(), inputHandler.getInputCol(), 
							ERROR, "The file ended while '%s' still had controll over the preprocessor. Level(s) in: %d", 
							controllers.peek().getName(), controllers.size());
				return false;
			}
			
			//Keep going until we actually set the output char!
			if(trySetCurrentChar(inputHandler.getInputChar(), inputHandler.getInputLine(), inputHandler.getInputCol())) {
				boolean allowed = true;
				for(int i = controllers.size()-1; i >= 0; i--) {
					if(!controllers.get(i).allowAdvanceToReturn(this, inputHandler)) {
						allowed = false;
						break;
					}
				}
				if(allowed)
					return true;
			}
		}
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
				if(inputHandler.getInputChar()=='\n') {
					inputHandler.pushCurrentChar();
					break;
				}
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
		
		String directive = pickUpPreProcDirective(inputHandler); //After, the char immediately after the directive is on the stack
		
		for(int i = controllers.size()-1; i >= 0; i--) {
			if(!controllers.get(i).allowDirectiveToHappen(directive, line, col, this, inputHandler))
				return false;
		}
		
		for(DirectiveHandler handler:DIRECTIVE_HANDLERS) {
			int result = handler.handleDirective(directive, line, col, this, inputHandler);
			if(result != DirectiveHandler.CANT_HANLDE_DIRECTIVE)
				return false;
		}
		
		log(getFile(), line, col, ERROR, "Found directive ' #%s ' that the preprocessor can't handle!", directive);
		
		return false;
	}
	
	public static String pickUpPreProcDirective(InputHandler inputHandler) {
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
	
	public void giveUpControlTo(PreProcessorController controller) {
		this.controllers.push(controller);
	}
	
	public void popBackControll() {
		this.controllers.pop();
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
	
	public class InputHandler implements Macro.CharStack {
		
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
		
		public void pushCurrentChar() {
			pushBufferedChar(inputChar, inputLine, inputCol);
		}
		
		public void pushBufferedChar(char c, int line, int col) {
			bufferedInputChars.push(c);
			bufferedInputLines.push(line);
			bufferedInputColms.push(col);
		}
		
		public void pushMultipleChars(String text, int line, int col) {
			for(int i = text.length()-1; i >= 0; i--) {
				pushBufferedChar(text.charAt(i), line, col);
			}
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
		public boolean addMacro(Macro macro) {
			logAssert(macro != null);
			return macros.lastElement().tryAddMacro(macro);
		}
		public Macro getMacro(String name) {
			for(int i = macros.size()-1; i >= 0; i--) {
				Macro m = macros.get(i).getMacro(name);
				if(m != null)
					return m;
			}
			return null;
		}
		public void close() {
			fileInput.close();
		}

		@Override
		public void pushMacroMap(MacroMap map) {
			macros.push(map);
		}
		
		public void popMacroMap() {
			macros.pop();
			if(macros.isEmpty()) {
				log(FATAL_ERROR, "The macro map stack in the pre processor is somehow empty");
			}
		}
		
		@Override
		public void pushMacroMapPopCommand(int line, int col) {
			pushBufferedChar('>', line, col);
			pushBufferedChar('<', line, col);
			for(int i = PopMacroStackDirectiveHandler.POP_MACRO_STACK_DIRECTIVE.length()-1; i>=0; i--) {
				pushBufferedChar(PopMacroStackDirectiveHandler.POP_MACRO_STACK_DIRECTIVE.charAt(i), line, col);
			}
			pushBufferedChar(TextParserUtil.POUND_SYMBOL, line, col);
		}
	}
}
