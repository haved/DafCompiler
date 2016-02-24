package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenLocation;
import me.haved.dafParser.semantic.Definition;
import me.haved.dafParser.semantic.KnownOfClass;

import static me.haved.dafParser.LogHelper.*;

public class MainInputFile extends InputFile {

	private HashSet<String> takenDefinitonNames;
	
	protected ArrayList<Definition> importedDef;
	protected ArrayList<Definition> usedDef;
	protected ArrayList<KnownOfClass> usedClasses;
	
	public MainInputFile(File inputFile, String infileName) {
		super(inputFile, infileName);
		
		importedDef = new ArrayList<>();
		usedDef = new ArrayList<>();
		usedClasses = new ArrayList<>();
		
		takenDefinitonNames = new HashSet<>();
	}
	
	@Override
	protected void goThroughTokens(ArrayList<Token> tokens) {
		
	}
	
	@Override
	protected void onUsingFound(TokenLocation location, InputFile file) {
		try {
			file.parse();
		}
		catch (AlreadyParsingException e) {
			log(location.getErrorString(), ERROR, "Recursive importing discovered of file %s!", file.infileName);
			return;
		}
		if(file.isParsed()) {
			ArrayList<Definition> definitions = file.getDefinitions();
			for(Definition def:definitions)
				if(def.isPublic())
					addUsedDefinition(def, location);
		}
	}
	
	private void addUsedDefinition(Definition def, TokenLocation location) {
		if(takenDefinitonNames.contains(def.getName())) {
			log(location.getErrorString(), ERROR, "Definition from '%s' was already defined", def.getStartLocation().getErrorString());
		} else {
			usedDef.add(def);
			takenDefinitonNames.add(def.getName());
		}
	}
}
