package me.haved.daf.data;

import java.io.PrintStream;

public interface Definition extends Node {
	void print(PrintStream out);
	
	boolean isPublic();
}
