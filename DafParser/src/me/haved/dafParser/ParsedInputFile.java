package me.haved.dafParser;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.HashMap;

import me.haved.dafParser.lexical.LexicalParser;
import me.haved.dafParser.node.Definition;
import me.haved.dafParser.node.Inline;
import me.haved.dafParser.node.RootNode;
import me.haved.dafParser.semantic.SemanticParser;

import static me.haved.dafParser.LogHelper.*;

public class ParsedInputFile {
	
	private static final HashMap<String, RootNode> parsedNodes = new HashMap<String, RootNode>(); 
	
	private File inputFile;
	private String infileName;
	private RootNode root;
	
	public ParsedInputFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
	}
	
	public void parse() throws Exception {
		logAssert(root == null, "Trying to parse an already parsed file!");
		logAssert(inputFile.isFile(), "ParsedInputFile got a file that doesn't exist! Should never happen!");
		String fileId = inputFile.getCanonicalPath();
		if(parsedNodes.containsKey(fileId)) {
			log(MESSAGE, "The file %s was already parsed. Using that node.", infileName);
			root = parsedNodes.get(fileId);
			return;
		}
		
		LexicalParser lexer = new LexicalParser(inputFile, infileName);
		lexer.parse();
		SemanticParser semantic = new SemanticParser(lexer.getTokens());
		semantic.parse();
		root = semantic.getRootNode();
		
		parsedNodes.put(fileId, root);
	}
	
	public void writeToCppAndHeader(File cppFile, File headerFile) throws Exception {
		logAssert(root != null, "Trying to write a non parsed daf file to .cpp and .h");
		log(infileName, MESSAGE, "ParsedInputFile writing to files '%s' and '%s'", cppFile.getName(), headerFile.getName());
		
		try (PrintWriter headerOut = new PrintWriter(new FileOutputStream(headerFile))) {
			writeToHeader(headerOut);
			headerOut.flush();
			headerOut.close();
		}
		
		try (PrintWriter cppOut = new PrintWriter(new FileOutputStream(cppFile))) {
			writeToCpp(cppOut, headerFile.getName());
			cppOut.flush();
			cppOut.close();
		}
	}
	
	private void writeToHeader(PrintWriter out) {
		out.println("#pragma once");
		for(Definition definition:root.getDefinitions()) {
			if(definition instanceof Inline) {
				Inline code = (Inline) definition;
				if(code.isHeader())
					out.print(code.getText());
			}
		}
	}
	
	private void writeToCpp(PrintWriter out, String headerName) {
		out.printf("#include \"%s\"%n", headerName);
		for(Definition definition:root.getDefinitions()) {
			if(definition instanceof Inline) {
				Inline code = (Inline) definition;
				if(code.isSource())
					out.print(code.getText());
			}
		}
	}
}