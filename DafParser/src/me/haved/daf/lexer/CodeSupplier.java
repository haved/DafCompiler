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
		current = fileText.getCurrentChar();
		line = fileText.getCurrentLine();
		col = fileText.getCurrentCol();
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
