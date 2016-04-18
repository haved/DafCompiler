package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

import java.util.Stack;

public class CodeSupplier {
	
	private RegisteredFile file;
	private MacroMap macros;
	
	private FileTextSupplier fileText;
	
	private char current;
	private int line;
	private int col;
	
	private Stack<Character> charBuffer;
	private Stack<Integer> lineNumBuffer;
	private Stack<Integer> colNumBuffer;
	
	public CodeSupplier(RegisteredFile file, MacroMap macros) {
		this.file = file;
		this.macros = macros;
		fileText = new FileTextSupplier(file);
		charBuffer = new Stack<>();
		lineNumBuffer = new Stack<>();
		colNumBuffer = new Stack<>();
		
		if(!fileText.hasChar())
			log(ASSERTION_FAILED, "new FileTextSupplier was created, but has NO chars!");
		
		if(!trySetCurrentChar(fileText.getCurrentChar(), fileText.getCurrentCol(), fileText.getCurrentLine())) //This should never be false because '\n'
			log(ASSERTION_FAILED, "new FileTextSupplier gave a char that didn't fill the code supplier");
	}
	
	private char inputChar;
	private int inputLine;
	private int inputCol;
	
	public boolean advance() {
		while(true) {
			if(!advanceInput())
				return false;
			if(trySetCurrentChar(inputChar, inputLine, inputCol)) //If we didn't add a letter, try again!
				return true;
		}
	}
	
	private boolean advanceInput() {
		if(!charBuffer.empty()) {
			inputChar = charBuffer.pop();
			inputLine = lineNumBuffer.pop();
			inputCol  = colNumBuffer.pop();
			return true;
		} else if(fileText.advance()) {
			inputChar = fileText.getCurrentChar();
			inputLine = fileText.getCurrentLine();
			inputCol  = fileText.getCurrentCol();
			return true;
		} else
			return false;
	}
	
	private boolean trySetCurrentChar(char c, int line, int col) {
		if(c=='/')
			return checkComments(c);
		else if(c=='#')
			return doMacroAndFlowChecks(c);
			
		return forceSetCurrentChar(c, line, col);
	}
	
	private boolean forceSetCurrentChar(char c, int line, int col) {
		this.current = c;
		this.line = line;
		this.col = col;
		return true;
	}
	
	private boolean checkComments(char firstChar) {
		int firstLine = inputLine;
		int firstCol  = fileText.getCurrentCol ();
		
		if(!advanceInput())
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
			
		char next = inputChar;
		if(next == '/') { //One line comment!
			while(true) {
				if(!advanceInput())
					return false;
				if(inputChar=='\n')
					return forceSetCurrentChar(inputChar, inputLine, inputCol);
			}
		} else if(next=='*') {
			boolean lastStar=false;
			while(true) {
				if(!advanceInput())
					return false;
				char c = inputChar;
				if(lastStar&&c=='/')
					return false;
				lastStar = c=='*';
			}
		}
		
		pushBufferedInputChar(next, inputLine, inputCol);
		return forceSetCurrentChar(firstChar, firstLine, firstCol);
	}
	
	private boolean doMacroAndFlowChecks(char firstChar) {
		int firstLine = inputLine;
		int firstCol = inputCol;
		
		if(!advanceInput())
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		
		char nameStartChar = inputChar;
		if(!TextParserUtil.isStartOfIdentifier(nameStartChar)) { //So we need to set the pound symbol, and push the next letter
			pushBufferedInputChar(nameStartChar, inputLine, inputCol);
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		}
		
		StringBuilder identifierBuilder = new StringBuilder();
		identifierBuilder.append(nameStartChar);
		
		while(true) {
			if(!advanceInput()) { //We need to add all the shit back to the stack, and write the pound symbol to the current char
				for(int i = identifierBuilder.length()-1; i>=0; i--)
					pushBufferedInputChar(identifierBuilder.charAt(i), firstLine, firstCol+i+1);
				return forceSetCurrentChar(firstChar, firstLine, firstCol);
			}
			char c = inputChar;
			if(TextParserUtil.isIdentifierChar(c))
				identifierBuilder.append(c);
			else break;
		}
		
		String identifier = identifierBuilder.toString();
		
		int compilerFlagHandeling = handleCompilerFlag(identifier);
		if(compilerFlagHandeling == NEXT_CHAR_SET)//if this returns false, add the pound symbol and stuff back.
			return true;
		else if(compilerFlagHandeling == USE_STACK_OR_FILE_FOR_NEXT)
			return false;
		else { //WE push the identifier and force set the pound symbol
			for(int i = identifier.length()-1; i>=0; i--)
				pushBufferedInputChar(identifier.charAt(i), firstLine, firstCol+i+1);
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		}
	}
	
	/** Handles whatever happens when code is done. The current char at the time of calling is 
	 * 	the first char after the identifier
	 * 
	 * @param identifier the name of the flag
	 * @return the key saying what to do with the current char and stack
	 */
	
	private static final int PUSH_IDENTIFIER_AGAIN = 0;
	private static final int USE_STACK_OR_FILE_FOR_NEXT = 1;
	private static final int NEXT_CHAR_SET = 2;
	
	private int handleCompilerFlag(String identifier) {
		if(identifier.equalsIgnoreCase("macro")) {
			int line = inputLine;
			int col = inputCol;
			if(!handleMacroDefinition()) {
				log(getFileName(), line, col, ERROR, "Macro definition failed");
			}
			return USE_STACK_OR_FILE_FOR_NEXT; //Means the fileText is advanced after this.
		}
		//The letter after the macro name is our responsibility to push back
		pushBufferedInputChar(inputChar, inputLine, inputCol);
		return PUSH_IDENTIFIER_AGAIN;
	}
	
	/**When called, currentChar is the newline or $ of the macro definition
	 * 
	 * @return
	 */
	private boolean handleMacroDefinition() {
		if(!TextParserUtil.isNormalWhitespace(inputChar)) {
			log(getFileName(), inputLine, inputCol, ERROR, "'#macro' must be followed by a space or tab, not '%c'", inputChar);
			return false;
		}
		
		StringBuilder macroDef = new StringBuilder();
		
		boolean encloseMacro = false;
		
		while(true) {
			if(!advanceInput()) {
				log(getFileName(), inputLine, inputCol, ERROR, "File ended before macro definition!");
				return false;
			}
			char c = inputChar;
			macroDef.append(c);
			if(c==TextParserUtil.ENCLOSE_MACRO) {
				if(!encloseMacro) {
					encloseMacro = true;
				}
				else {
					break;
				}
			}
			else if(c==TextParserUtil.END_OF_LINE && !encloseMacro) {
				break;
			}
		}
		
		Macro macro = Macro.getMacroFromString(macroDef.toString());
		if(macro == null)
			return false;
		
		macros.tryAddMacro(macro);
		return true;
	}
	
	private void pushBufferedInputChar(char c, int line, int col) {
		charBuffer.push(c);
		lineNumBuffer.push(line);
		colNumBuffer.push(col);
	}
	
	public String getFileName() {
		return file.fileName;
	}
	
	public void close() {
		fileText.close();
	}
	
	public char getCurrentChar() {
		return current;
	}
	
	public int getCurrentLine() {
		return line;
	}
	
	public int getCurrentCol() {
		return col;
	}
}
