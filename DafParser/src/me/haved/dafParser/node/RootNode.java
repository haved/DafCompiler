package me.haved.dafParser.node;

import java.io.PrintWriter;
import java.util.ArrayList;

import me.haved.dafParser.ParsedInputFile;
import me.haved.dafParser.semantic.DefinitionMaker;
import me.haved.dafParser.semantic.TokenPosition;

import static me.haved.dafParser.LogHelper.*;

public class RootNode extends Node {

	private ArrayList<Definition> definitions;
	private ArrayList<ParsedInputFile> includedFiles;
	
	public RootNode() {
		definitions = new ArrayList<>();
		includedFiles = new ArrayList<>();
	}
	
	@Override
	public void FillFromTokens(TokenPosition tokens) {
		for (;tokens.hasMore();) {
			Definition definition = DefinitionMaker.MakeDefinition(tokens);
			if(definition == null)
				log(DEBUG, "Definition returned from DefinitionMaker was null");
			else
				definitions.add(definition);
			tokens.next();
		}
	}
	
	@Override
	public void PrintToCppWriter(PrintWriter writer) {
		for(Definition definition:definitions) {
			definition.PrintToCppWriter(writer);
			writer.println();
		}
	}
	
	@Override
	public void PrintToHeaderWriter(PrintWriter writer) {
		for(Definition definition:definitions) {
			definition.PrintToHeaderWriter(writer);
			writer.println();
		}
	}
	
	@Override
	public String getName() {
		return "Root Node";
	}

	@Override
	public String compileSubnodesToString() {
		StringBuilder builder = new StringBuilder("(RootNode, ");
		for(int i = 0; i < definitions.size(); i++) {
			if(i>0)
				builder.append(", ");
			builder.append(definitions.get(i).compileToString());
		}
		builder.append(" )");
		return builder.toString();
	}
	
	public ArrayList<ParsedInputFile> getIncludedFiles() {
		return includedFiles;
	}
	
	public ArrayList<Definition> getDefinitions() {
		return definitions;
	}
}
