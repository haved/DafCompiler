package me.haved.dafParser.node;

public class Inline extends Node implements Definition {
	
	private boolean header;
	private String text;
	
	public Inline(boolean header, String text) {
		this.header = header;
		this.text = text;
	}
	
	@Override
	public String getName() {
		return "Inline";
	}

	@Override
	public String compileSubnodesToString() {
		return String.format("%s, %s", header?"Header":"Source", text);
	}
	
	public boolean isHeader() {
		return header;
	}
	
	public boolean isSource() {
		return !header;
	}
	
	public String getText() {
		return text;
	}
}
