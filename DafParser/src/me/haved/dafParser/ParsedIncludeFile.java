package me.haved.dafParser;

import java.util.HashMap;

import me.haved.dafParsed.node.RootNode;

public class ParsedIncludeFile {
	private static HashMap<String, RootNode> parsedIncludeNodes = new HashMap<String, RootNode>();
	
	private String fileName;
	private RootNode root;
	
	public ParsedIncludeFile(String fileName) {
		this.fileName = fileName;
		if(parsedIncludeNodes.containsKey(fileName)) {
			this.root = parsedIncludeNodes.get(fileName);
		}
	}
	
	public void parse() {
		root = new RootNode();
		parsedIncludeNodes.put(fileName, root);
	}
	
	public boolean isParsed() {
		return root != null;
	}
}
