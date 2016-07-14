package me.haved.daf.data.definition;

import java.io.PrintStream;

import me.haved.daf.data.Node;

public interface Definition extends Node {
	void print(PrintStream out);
	
	boolean isPublic();
}
