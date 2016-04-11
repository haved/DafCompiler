package me.haved.daf.lexer;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class FileTextSupplier {
	private RegisteredFile file;
	private BufferedReader reader;
	
	private char current;
	private int line;
	private int col;
	
	private boolean finalNewline = false;
	private boolean done = false;
	
	public FileTextSupplier(RegisteredFile file) {
		this.file = file;
		this.current = '\n';
		this.line = 0;
		this.col = 0;
		
		if(!file.fileObject.canRead())
			log(ASSERTION_FAILED, "File given to FileTextSupllier was unreadable: %s", file.fileName);
		try {
			reader = new BufferedReader(new FileReader(file.fileObject));
			if(!reader.ready())
				log(ASSERTION_FAILED, "Created reader for file was never ready: %s", file.fileName);
		}
		catch(Exception e) {
			log(ASSERTION_FAILED, "Failed to open file for reading in FileTextSupplier: '%s'", file.fileName);
		}
	}
	
	public void close() {
		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public boolean advance() throws IOException {
		if(done | finalNewline) {
			done = true;
			return false;
		}
		
		if(current == '\n') {
			line++;
			col = 0;
		}
		col += current=='\t' ? 4 : 1;
		
		int nextChar = reader.read();
		if(nextChar == -1) {
			finalNewline = true;
			current = '\n';
		}
		else
			current = (char) nextChar;
			
		return true;
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
	
	public String getFileName() {
		return file.fileName;
	}
}
