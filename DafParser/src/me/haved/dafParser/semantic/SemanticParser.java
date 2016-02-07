package me.haved.dafParser.semantic;

import java.io.File;
import java.util.ArrayList;

import me.haved.dafParser.ParsedInputFile;
import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenType;
import me.haved.dafParser.node.RootNode;

import static me.haved.dafParser.LogHelper.*;

public class SemanticParser {
	
	private ArrayList<Token> tokens;
	private RootNode node;
	
	private ArrayList<ParsedInputFile> importedFiles = new ArrayList<>();
	private ArrayList<ParsedInputFile> usedFiles	 = new ArrayList<>();

	public SemanticParser(ArrayList<Token> tokens) {
		this.tokens = tokens;
	}
	
	public void parseIncludedFiles(String folderName) throws Exception {
		for(int i = 0; i < tokens.size(); i++) {
			TokenType type = tokens.get(i).getType();
			if(type==TokenType.DAF_IMPORT | type==TokenType.DAF_USING) {
				Token token = tokens.get(i);
				ParsedInputFile file = ParsedInputFile.makeInputFileInstance(new File(String.format("%s/%s", folderName, token.getText())), token.getText());
				if(file==null)
					continue;
				file.parse();
				
			}
		}
	}
	
	public ArrayList<ParsedInputFile> getImportedFiles() {
		return importedFiles;
	}
	
	public ArrayList<ParsedInputFile> getUsedFiles() {
		return usedFiles;
	}
	
	public void parse() {
		logAssert(tokens != null, "The tokens passed to the Semantic Parser were null");
		log(MESSAGE, "Starting semantic parsing with %d tokens", tokens.size());
		node = new RootNode();
		TokenPosition position = new TokenPosition(tokens);
		node.FillFromTokens(position);
		if(position.position < tokens.size()) {
			log(WARNING, "It would seem not all tokens were read!");
		}
		
		log(MESSAGE, node.compileSubnodesToString());
		
		terminateIfErrorsLogged();
		log(MESSAGE, "Finished semantic parsing");
	}
	
	public RootNode getRootNode() {
		logAssert(node != null, "The Semantic Parsers hasn't parsed the file yet!");
		return node;
	}
}
