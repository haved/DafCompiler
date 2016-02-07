package me.haved.dafParser;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;

import me.haved.dafParser.lexical.LexicalParser;
import me.haved.dafParser.node.RootNode;
import me.haved.dafParser.semantic.SemanticParser;

import static me.haved.dafParser.LogHelper.*;

public class ParsedInputFile {
	private File inputFile;
	private String infileName;
	
	private RootNode root;
	private ArrayList<ParsedInputFile> includedFiles; 
	
	public ParsedInputFile() {
		logAssert(false, "Empty ParsedInputFile constructor called");
	}
	
	private ParsedInputFile(File inputFile, String infileName) {
		this.inputFile = inputFile;
		this.infileName = infileName;
	}
	
	public void parse() throws Exception {
		logAssert(!isParsed(), "Trying to parse an already parsed file!");
		logAssert(inputFile.isFile(), "ParsedInputFile.parse() got a file that doesn't exist! Should never happen!");
		
		LexicalParser lexer = new LexicalParser(inputFile, infileName);
		lexer.parse();
		SemanticParser semantic = new SemanticParser(lexer.getTokens());
		includedFiles = semantic.parseIncludedFiles(inputFile.getParent());
		semantic.parse();
		root = semantic.getRootNode();
	}
	
	public void checkSyntax() {
		
	}
	
	public void writeToCppAndHeader(File cppFile, File headerFile) throws Exception {
		logAssert(isParsed(), "Trying to write a non parsed daf file to .cpp and .h");
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
	
	public boolean isParsed() {
		return root != null;
	}
	
	private void writeToHeader(PrintWriter out) {
		out.println("#pragma once");
		root.PrintToHeaderWriter(out);
	}
	
	private void writeToCpp(PrintWriter out, String headerName) {
		out.printf("#include \"%s\"%n", headerName);
		root.PrintToCppWriter(out);
	}
	
	
	private static final HashMap<String, ParsedInputFile> parsedFiles = new HashMap<String, ParsedInputFile>();
	
	public static ParsedInputFile makeInputFileInstance(File inputFile, String infileName) {
		try {
			String infileId = inputFile.getCanonicalPath();
			if(parsedFiles.containsKey(infileId)) {
				return parsedFiles.get(infileId);
			}
			else {
				ParsedInputFile instance = new ParsedInputFile(inputFile, infileName);
				parsedFiles.put(infileId, instance);
				return instance;
			}
		} catch(Exception e) {
			log(infileName, ERROR, "Failed to make instance of ParsedInputFile");
			e.printStackTrace(out);
			terminateIfErrorsLogged();
			return null;
		}
	}
}