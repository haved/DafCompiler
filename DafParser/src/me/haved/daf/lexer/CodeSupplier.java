package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

import java.util.Stack;

public class CodeSupplier {
	
	private RegisteredFile file;
	private FileTextSupplier fileText;
	
	private char current;
	private int line;
	private int col;
	
	private Stack<Character> charBuffer;
	private Stack<Integer> lineNumBuffer;
	private Stack<Integer> colNumBuffer;
	
	public CodeSupplier(RegisteredFile file) {
		this.file = file;
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
		
		//I there wasn't a comments
		addBufferedChar(next, fileText.getCurrentLine(), fileText.getCurrentCol());
		return forceSetCurrentChar(firstChar, firstLine, firstCol);
	}
	
	private void addBufferedChar(char c, int line, int col) {
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
