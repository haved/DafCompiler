package me.haved.daf.lexer;

import java.io.IOException;

public class FIleCodeSupplier implements Supplier {

	private FileTextSupplier file;
	
	public FIleCodeSupplier(FileTextSupplier file) {
		this.file = file;
	}
	
	@Override
	public boolean isOpen() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean hasChar() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public char getCurrentChar() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getCurrentLine() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getCurrentCol() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public boolean advance() throws IOException {
		// TODO Auto-generated method stub
		return false;
	}

}
