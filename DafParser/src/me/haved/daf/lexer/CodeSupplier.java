package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class CodeSupplier {
	
	private RegisteredFile file;
	private FileTextSupplier fileText;
	
	private char current;
	private int line;
	private int col;
	
	private char[] bufferedChars;
	private int[] bufferedLineNums;
	private int[] bufferedColNums;
	private int bufferIndex = 0; //The index of the yet to be read buffered char
	
	public CodeSupplier(RegisteredFile file) {
		this.file = file;
		fileText = new FileTextSupplier(file);
		
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
		if(bufferedChars != null && bufferIndex < bufferedChars.length) {
			fc.c = bufferedChars[bufferIndex];
			fc.line = bufferedLineNums[bufferIndex];
			fc.col = bufferedColNums[bufferIndex];
			bufferIndex++;
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
			return false;
		
		current = c;
		this.line = line;
		this.col = col;
		
		return true;
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
