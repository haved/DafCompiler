package me.haved.daf.lexer;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class FileTextSupplier implements Supplier {
	public static int DEFAULT_TAB_WIDTH = 4;
	
	private int tabWidth;
	
	private RegisteredFile file;
	private BufferedReader in;
	
	private char current;
	private int currentLine;
	private int currentCol;
	private boolean finalNewline;
	private boolean done;
	
	public FileTextSupplier(RegisteredFile file) throws FileNotFoundException {
		this(file, DEFAULT_TAB_WIDTH);
	}
	
	public FileTextSupplier(RegisteredFile file, int tabWidth) throws FileNotFoundException {
		this.tabWidth = tabWidth;
		this.file = file;
		if(file.fileObject == null || !file.fileObject.exists()) {
			log(ASSERTION_FAILED, "The file passed to FileTextSupplier didn't exist");
		}
		in = new BufferedReader(new FileReader(file.fileObject));
		
		current = '\n';
		currentLine = 0;
		currentCol = 0;
		finalNewline = false;
		done = false;
	}
	
	public void close() throws IOException {
		in.close();
	}
	
	public boolean hasChar() {
		return !done; 
	}
	
	public char getCurrentChar() {
		if(!hasChar())
			log(FATAL_ERROR, "Asking %s for current char when hasChar() is false!", toString());
		return current;
	}
	
	public int getCurrentLine() {
		return currentLine;
	}
	
	public int getCurrentCol() {
		return currentCol;
	}
	
	public boolean advance() throws IOException {
		if(current == '\n') {
			currentLine++;
			currentCol = 0;
		}
		
		currentCol += current == '\t' ? tabWidth : 1; //If the current char is a tab, it is (probably) 4 long instead of the normal 1
				
		if(finalNewline) {
			done = true;
			return false;
		}
		
		int i = in.read(); //If the file is over, add an extra newline just to be sure
		if(i < 0) { //No more actual file content, but we keep going for another advance() call until hasChar() returns false
			current = '\n';
			in.close();
			finalNewline = true;
		} else
			current = (char) i;
		return true;
	}
	
	public RegisteredFile getFile() {
		return file;
	}
	
	@Override
	public String toString() {
		return String.format("FileTextSupplier(%s)", file.fileName);
	}
}
