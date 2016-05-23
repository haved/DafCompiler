package me.haved.daf.data;

import java.io.PrintStream;

public interface Definition {
	void print(PrintStream out);
	
	boolean isPublic();
}
