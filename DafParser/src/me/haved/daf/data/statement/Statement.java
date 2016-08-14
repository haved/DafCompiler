package me.haved.daf.data.statement;

import java.io.PrintWriter;

import me.haved.daf.data.Node;

public interface Statement extends Node {
	public boolean isValidStatement();

	public void codegenStatementCpp(PrintWriter cpp);
}
