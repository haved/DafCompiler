package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

import java.io.BufferedReader;
import java.io.IOException;

public class CodeSupplier {
	
	private RegisteredFile file;
	private BufferedReader fileReader;
	
	private char[] bufferedChars;
	private int[] bufferedLineNums;
	private int[] bufferedColNums;
	private int bufferIndex = 0;
	
	public CodeSupplier(RegisteredFile file) {
		this.file = file;
		
	}
	
	public String getFileName() {
		return file.fileName;
	}
	
	public void close() {
		try {
			fileReader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
