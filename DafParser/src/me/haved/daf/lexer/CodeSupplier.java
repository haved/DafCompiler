package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

public class CodeSupplier {
	
	private RegisteredFile file;
	
	public CodeSupplier(RegisteredFile file) {
		this.file = file;
	}
	
	public String getFileName() {
		return file.fileName;
	}
}
