package me.haved.daf.lexer;

import java.util.ArrayList;

public class TextBufferer {
	
	private CodeSupplier supplier;
	private boolean open; //If the CodeSupplier is open
	
	private ArrayList<Character> letters  = new ArrayList<>();
	private ArrayList<Integer>   lineNums = new ArrayList<>();
	private ArrayList<Integer>   colNums  = new ArrayList<>();
	private int letterIndex;
	
	public TextBufferer(CodeSupplier supplier) {
		this.supplier = supplier;
		this.open = true;
		this.letterIndex = 0;
		
		letters .add(supplier.getCurrentChar());
		lineNums.add(supplier.getCurrentLine());
		colNums .add(supplier.getCurrentCol ());
	}
	
	public boolean hasChar() {
		return letterIndex < letters.size();
	}
	
	public char getCurrentChar() {
		return letters.get(letterIndex);
	}
	
	public int getCurrentLine() {
		return lineNums.get(letterIndex);
	}
	
	public int getCurrentCol() {
		return colNums.get(letterIndex);
	}
}
