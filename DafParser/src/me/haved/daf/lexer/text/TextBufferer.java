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
		
		setOnlyFirstLetter();
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
	
	/** Sets the start to the current char + saveAmount and removes anything prior
	 * 
	 * @param saveAmount the amount of chars before or after the current one to save
	 */
	public void setNewStart(int saveAmount) {
		
		if(saveAmount < 0)
			for(int i = 0; i > saveAmount; i--) {
				if(!advance()) {
					log(WARNING, "File ended before the textBufferer could skip %d chars", saveAmount);
					break;
				}
			}
		else if(saveAmount != 0)
			log(ASSERTION_FAILED, "TextBuffer asked to setNewStart saving %d letters. Has to be 0 or less in this implementation!", saveAmount);
			
		setOnlyFirstLetter();
		letterIndex = 0;
	}
	
	private void setOnlyFirstLetter() {
		letters .clear();
		lineNums.clear();
		colNums .clear();
		letters .add(supplier.getCurrentChar());
		lineNums.add(supplier.getCurrentLine());
		colNums .add(supplier.getCurrentCol ());
	}
}
