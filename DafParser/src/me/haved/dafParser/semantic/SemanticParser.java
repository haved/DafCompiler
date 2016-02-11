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
		importedFiles = new ArrayList<>();
		usedFiles = new ArrayList<>();
	}
	
	public void parseIncludedFiles(String folderName) throws Exception { //Another use for in scope functions in here :D
		for(int i = 0; i < tokens.size(); i++) {
			TokenType type = tokens.get(i).getType();
			if(type==TokenType.DAF_IMPORT | type==TokenType.DAF_USING) {
				Token token = tokens.get(i);
				File include = new File(String.format("%s/%s", folderName, token.getText()));
				if(!include.isFile()) {
					log(token.getErrorLoc(), ERROR, "File not found: '%s'", token.getText());
				}
				ParsedInputFile file = ParsedInputFile.makeInputFileInstance(include, token.getText(), false);
				if(file==null)
					continue;
				if(file.isParsing() & type==TokenType.DAF_IMPORT) {
						log(token.getErrorLoc(), ERROR, "The file %s is already parsing when imported. Not good, man");
				}
				else if(!file.isParsing())
					file.parse();
				if(!file.isParsed()) //Error
					continue;
				
				tryAddParsedFile(file, type==TokenType.DAF_IMPORT ? IMPORT : USING); //Type says if file was imported or used
				
				ArrayList<ParsedInputFile> fileImports = file.getImportedFiles();
				ArrayList<ParsedInputFile> fileUsings  = file.getUsedFiles();
				
				for(ParsedInputFile imported:fileImports) { //For each file the included file has imported
					tryAddParsedFile(imported, type==TokenType.DAF_IMPORT ? IMPORT : USING);
				}
				for(ParsedInputFile used:fileUsings) { //For each file the included file has used
					tryAddParsedFile(used, USING);
				}
			}
		}
	}
	
	private static final int IMPORT = 0;
	private static final int USING = 1;
	public void tryAddParsedFile(ParsedInputFile file, int includeType) {
		if(includeType == IMPORT) {
			if(!importedFiles.contains(file))
				importedFiles.add(file);
			if(usedFiles.contains(file)) // A file can't be both imported and used. Import trumps used
				usedFiles.remove(file); 
		}
		else if(includeType == USING) {
			if(!importedFiles.contains(file) && !usedFiles.contains(file))
				usedFiles.add(file);
		}
		else
			logAssert(false, "Unkown include type in Semantic Parser");
	}
	
	public ArrayList<ParsedInputFile> getImportedFiles() {
		return importedFiles;
	}
	
	public ArrayList<ParsedInputFile> getUsedFiles() {
		return usedFiles;
	}
	
	public void parsePubDefinitions() {
		logAssert(tokens != null, "The tokens passed to the Semantic Parser were null");
		log(MESSAGE, "Starting semantic parsing of public declarations with %d tokens", tokens.size());
		
		node = new RootNode();
		
		log(MESSAGE, node.compileSubnodesToString());
		terminateIfErrorsLogged();
		log(MESSAGE, "Finished semantic parsing of public declarations");
	}
	
	public void parseAll() {
		logAssert(tokens != null, "The tokens passed to the Semantic Parser were null");
		log(MESSAGE, "Starting semantic parsing of whole file with %d tokens", tokens.size());
		node = new RootNode();
		TokenPosition position = new TokenPosition(tokens);
		node.FillFromTokens(position);
		if(position.position < tokens.size()) {
			log(WARNING, "It would seem not all tokens were read!");
		}
		
		log(MESSAGE, node.compileSubnodesToString());
		terminateIfErrorsLogged();
		log(MESSAGE, "Finished semantic parsing of whole file");
	}
	
	public RootNode getRootNode() {
		logAssert(node != null, "The Semantic Parsers hasn't parsed the file yet!");
		return node;
	}
}
