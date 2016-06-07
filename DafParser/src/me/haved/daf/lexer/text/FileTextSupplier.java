package me.haved.daf.lexer.text;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class FileTextSupplier implements TextSupplier {
	private RegisteredFile file;
	private BufferedReader reader;
	
	private char current;
	private int line;
	private int col;
	
	private boolean outOfChars = false;
	private boolean done = false;
	
	public FileTextSupplier(RegisteredFile file) {
		this.file = file;
		this.current = '\n';
		this.line = 0;
		this.col = 0;
		
		if(!file.getFileObject().canRead())
			log(ASSERTION_FAILED, "File given to FileTextSupllier was unreadable: %s", file.getFileName());
		try {
			reader = new BufferedReader(new FileReader(file.getFileObject()));
			if(!reader.ready())
				log(ASSERTION_FAILED, "Created reader for file was never ready: %s", file.getFileName());
		}
		catch(Exception e) {
			log(ASSERTION_FAILED, "Failed to open file for reading in FileTextSupplier: '%s'", file.getFileName());
		}
	}
	
	public void close() {
		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 *  Must have set current char to a new-line before false may ever be returned 
	 */
	public boolean advance() {
		try {
			if(done) {
				return false;
			}
			else if(outOfChars) {
				logAssert(current=='\n');
				done = true;
				close();
				return false;
			}
			
			if(current == '\n') {
				line++;
				col = 0;
			}
			col += current=='\t' ? 4 : 1;
			
			int nextChar = reader.read();
			if(nextChar == -1) {
				outOfChars = true;
				if(current == '\n') { //already newline
					done = true;
					close();
					return false;
				}
				current = '\n';
			}
			else
				current = (char) nextChar;
				
			return true;
		} catch(IOException e) {
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean hasChar() {
		return !done;
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
	
	public RegisteredFile getFile() {
		return file;
	}
}
