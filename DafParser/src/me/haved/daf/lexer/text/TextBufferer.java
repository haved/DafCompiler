package me.haved.daf.lexer.text;

import java.util.ArrayList;

import static me.haved.daf.LogHelper.*;

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
	
	public String getSourceName() {
		return supplier.getFileName();
	}
	
	public boolean advance() {
		if(letterIndex < letters.size()-1) {
			letterIndex++;
			return true;
		}
		
		if(open && supplier.advance()) {
			letters .add(supplier.getCurrentChar());
			lineNums.add(supplier.getCurrentLine());
			colNums .add(supplier.getCurrentCol ());
			letterIndex++;
			return true;
		}
		
		return open = false;
	}
	
	public void restoreToStart() {
		letterIndex = 0;
	}
	
	public void setNewStart(int saveAmount) {
		if(saveAmount == 0) {
			letters.clear();
			lineNums.clear();
			colNums.clear();
		} else 
			log(ASSERTION_FAILED, "TextBuffer asked to setNewStart saving %d letters. Has to be 0 in this implementation!", saveAmount);
		letterIndex = 0;
	}
}
