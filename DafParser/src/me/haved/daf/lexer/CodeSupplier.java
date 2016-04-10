package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class CodeSupplier {
	
	private RegisteredFile file;
	
	private FileChar[] bufferedFileChars;
	private int bufferIndex = 0;
	
	private FileChar currentFileChar = null;
	
	public CodeSupplier(RegisteredFile file) {
		this.file = file;
	}
	
	public String getFileName() {
		return file.fileName;
	}
	
	public FileChar getCurrentChar() {
		if(currentFileChar == null)
			log(ASSERTION_FAILED, "Asked CodeSupplier for current char when there were none!");
		return currentFileChar;
	}
	
	/**
	 * 
	 */
	public boolean advance() {
		if(bufferIndex <= bufferedFileChars.length) {
			currentFileChar = bufferedFileChars[bufferIndex];
			bufferIndex++;
			return true;
		}
		
		return false;
	}
}
