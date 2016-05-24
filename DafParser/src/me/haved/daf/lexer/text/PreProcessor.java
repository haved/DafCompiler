package me.haved.daf.lexer.text;

import java.util.Stack;

import me.haved.daf.RegisteredFile;

public class PreProcessor implements TextSupplier {
	
	private TextSupplier fileInput;
	private Stack<MacroMap> macros;

	private char outputChar;
	private int  outputLine;
	private int  outputCol;
	
	public PreProcessor(RegisteredFile file, MacroMap macros) {
		this.fileInput = new FileTextSupplier(file);
		
		this.macros = new Stack<>();
		this.macros.push(macros);
		
		trySetCurrentChar(fileInput.getCurrentChar(), fileInput.getCurrentLine(), fileInput.getCurrentCol());
	}
	
	@Override
	public boolean advance() {
		if(!fileInput.advance())
			return false;
		
		trySetCurrentChar(fileInput.getCurrentChar(), fileInput.getCurrentLine(), fileInput.getCurrentCol());
		return true;
	}
	
	private void trySetCurrentChar(char c, int line, int col) {
		if(c=='/'){} //Try doing comments and stuff
		else if(c=='#'){} //Try doing other things
		forceSetCurrentChar(c, line, col);
	}
	
	private void forceSetCurrentChar(char c, int line, int col) {
		this.outputChar = c;
		this.outputLine = line;
		this.outputCol = col;
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
