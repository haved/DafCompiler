package me.haved.dafParser.node;

import java.io.PrintWriter;

public class Inline extends Node implements Definition {
	
	public static final boolean HEADER = true;
	public static final boolean SOURCE = false;
	
	private boolean header;
	private String text;
	
	public Inline(boolean header, String text) {
		this.header = header;
		this.text = text;
	}
	
	@Override
	public void PrintToCppWriter(PrintWriter writer) {
		if(isSource()) {
			writer.println(text);
		}
	}
	
	@Override
	public void PrintToHeaderWriter(PrintWriter writer) {
		if(isHeader()) {
			writer.println(text);
		}
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
		return header==HEADER; // I know
	}
	
	public boolean isSource() {
		return header==SOURCE;
	}
	
	public String getText() {
		return text;
	}
}
