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
	private int bufferIndex = 0;
	
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
		fc.c++;
		fc.col++;
		return true;
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
