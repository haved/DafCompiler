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
	
	private FileChar fc = new FileChar();
	public boolean advance() {
		while(true) {
			if(!getNextChar(fc))
				return false;
			if(trySetCurrentChar(fc.c, fc.line, fc.col)) //If we didn't add a letter, try again!
				return true;
		}
	}
	
	private boolean getNextChar(FileChar fc) {
		if(!charBuffer.empty()) {
			fc.c = charBuffer.pop();
			fc.line = lineNumBuffer.pop();
			fc.col = colNumBuffer.pop();
			return true;
		} else if(fileText.advance()) {
			fc.c = fileText.getCurrentChar();
			fc.line = fileText.getCurrentLine();
			fc.col = fileText.getCurrentCol();
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
		int firstLine = fileText.getCurrentLine();
		int firstCol  = fileText.getCurrentCol ();
		
		if(!fileText.advance())
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
			
		char next = fileText.getCurrentChar();
		if(next == '/') { //One line comment!
			while(true) {
				if(!fileText.advance())
					return false;
				if(fileText.getCurrentChar()=='\n')
					return forceSetCurrentChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
			}
		} else if(next=='*') {
			boolean lastStar=false;
			while(true) {
				if(!fileText.advance())
					return false;
				char c = fileText.getCurrentChar();
				if(lastStar&&c=='/')
					return false;
				lastStar = c=='*';
			}
		}
		
		pushBufferedChar(next, fileText.getCurrentLine(), fileText.getCurrentCol());
		return forceSetCurrentChar(firstChar, firstLine, firstCol);
	}
	
	private boolean doMacroAndFlowChecks(char firstChar) {
		int firstLine = fileText.getCurrentLine();
		int firstCol = fileText.getCurrentCol();
		
		if(!fileText.advance())
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		
		char nameStartChar = fileText.getCurrentChar();
		if(!TextParserUtil.isStartOfIdentifier(nameStartChar)) { //So we need to set the pound symbol, and push the next letter
			pushBufferedChar(nameStartChar, fileText.getCurrentLine(), fileText.getCurrentCol());
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		}
		
		StringBuilder identifierBuilder = new StringBuilder();
		identifierBuilder.append(nameStartChar);
		
		while(true) {
			if(!fileText.advance()) { //We need to add all the shit back to the stack, and write the pound symbol to the current char
				for(int i = identifierBuilder.length()-1; i>=0; i--)
					pushBufferedChar(identifierBuilder.charAt(i), firstLine, firstCol+i+1);
				return forceSetCurrentChar(firstChar, firstLine, firstCol);
			}
			char c = fileText.getCurrentChar();
			if(TextParserUtil.isIdentifierChar(c))
				identifierBuilder.append(c);
			else break;
		}
		
		String identifier = identifierBuilder.toString();
		log(MESSAGE, "Found compiler token: %s", identifier);
		
		int compilerFlagHandeling = handleCompilerFlag(identifier);
		if(compilerFlagHandeling == NEXT_CHAR_SET)//if this returns false, add the pound symbol and stuff back.
			return true;
		else if(compilerFlagHandeling == USE_STACK_OR_FILE_FOR_NEXT)
			return false;
		else { //WE push the identifier and force set the pound symbol
			pushBufferedChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
			for(int i = identifier.length()-1; i>=0; i--)
				pushBufferedChar(identifier.charAt(i), firstLine, firstCol+i+1);
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
			int line = fileText.getCurrentLine();
			int col = fileText.getCurrentCol();
			if(!handleMacroDefinition()) {
				log(getFileName(), line, col, ERROR, "Macro definition failed");
			}
			return USE_STACK_OR_FILE_FOR_NEXT;
		}
		//The letter after the macro name is our responsibility to push back
		pushBufferedChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
		return PUSH_IDENTIFIER_AGAIN;
	}
	
	/**When called, currentChar is the char after macro
	 * 
	 * @return
	 */
	private boolean handleMacroDefinition() {
		
		if(TextParserUtil.isNormalWhitespace(fileText.getCurrentChar())) {
			log(getFileName(), fileText.getCurrentLine(), fileText.getCurrentCol(), ERROR, "'#macro' must be followed by a space or tab, not '%c'", fileText.getCurrentChar());
			return false;
		}
		
		return true;
	}
	
	private void pushBufferedChar(char c, int line, int col) {
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
	
	private static class FileChar {
		public char c;
		public int line, col;
	}
}
