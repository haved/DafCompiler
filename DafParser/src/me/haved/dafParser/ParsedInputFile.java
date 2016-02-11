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
	
	private static final int NOT_STARTED = 0;
	private static final int STARTED = 1;
	private static final int DONE = 0;
	
	private File inputFile;
	private String infileName;
	
	private RootNode root;
	private ArrayList<ParsedInputFile> importedFiles;
	private ArrayList<ParsedInputFile> usedFiles;
	private boolean parseItAll;
	
	private int parseProgress = NOT_STARTED;
	
	private ParsedInputFile(File inputFile, String infileName, boolean parseItAll) {
		this.inputFile = inputFile;
		this.infileName = infileName;
		this.parseItAll = parseItAll;
	}
	
	public void parse() throws Exception {
		log(infileName, INFO, "Parsing a file!");
		
		if(parseProgress!=NOT_STARTED)
			log(infileName, FATAL_ERROR, "Trying to parse an already parsing/parsed file!");
		logAssert(inputFile.isFile(), "ParsedInputFile.parse() got a file that doesn't exist! Should never happen!");
		
		parseProgress = STARTED;
		LexicalParser lexer = new LexicalParser(inputFile, infileName);
		lexer.parse();
		SemanticParser semantic = new SemanticParser(lexer.getTokens());
		String absolutePath = inputFile.getAbsolutePath();
		semantic.parseIncludedFiles(absolutePath.substring(0, absolutePath.lastIndexOf(File.separatorChar)));
		importedFiles = semantic.getImportedFiles();
		usedFiles = semantic.getUsedFiles();
		if(importedFiles.contains(this))
			log(infileName, ERROR, "This file was imported by one of it's imports. Doesn't work. Try \"#using\" instead.");
		terminateIfErrorsLogged();
		if(parseItAll)
			semantic.parseAll();
		else 
			semantic.parsePubDefinitions();
		root = semantic.getRootNode();
		parseProgress = DONE;
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
	
	public boolean isParsing() {
		return parseProgress == STARTED;
	}
	
	public boolean isParsed() {
		return parseProgress == DONE;
	}
	
	public ArrayList<ParsedInputFile> getImportedFiles() {
		return importedFiles;
	}
	
	public ArrayList<ParsedInputFile> getUsedFiles() {
		return usedFiles;
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
	
	public static ParsedInputFile makeInputFileInstance(File inputFile, String infileName, boolean parseItAll) {
		try {
			String infileId = inputFile.getCanonicalPath();
			if(parsedFiles.containsKey(infileId)) {
				return parsedFiles.get(infileId);
			}
			else {
				ParsedInputFile instance = new ParsedInputFile(inputFile, infileName, parseItAll);
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