package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenLocation;
import me.haved.dafParser.semantic.AlreadyParsingException;
import me.haved.dafParser.semantic.Definition;
import me.haved.dafParser.semantic.DefinitionFinder;
import me.haved.dafParser.semantic.IncludeDefinition;
import me.haved.dafParser.semantic.KnownOfClass;
import me.haved.dafParser.semantic.TokenDigger;

import static me.haved.dafParser.LogHelper.*;

public class MainInputFile extends InputFile {

	private HashSet<String> takenDefinitonNames;
	
	protected ArrayList<Definition> ownDefinitions;
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
		
		TokenDigger digger = new TokenDigger(tokens);
		
		while(digger.hasMore()) {
			Definition def = DefinitionFinder.getDefinition(digger);
			if(def==null)
				continue;
			ownDefinitions.add(def);
			if(def instanceof IncludeDefinition) {
				IncludeDefinition include = (IncludeDefinition) def;
				if(include.isImporting()) {
					onImportFound(include.getStartLocation(), getFileFromInclude(include.getStartLocation(), include.getFileName()));
				}
				else if(include.isUsing()) {
					onUsingFound(include.getStartLocation(), getFileFromInclude(include.getStartLocation(), include.getFileName()));
				} else
					fatal_error();
			}
		}
		
		terminateIfErrorsLogged();
	}
	
	protected void onImportFound(TokenLocation location, InputFile file) {
		try {
			file.parse();
			if(!file.isParsed()) {
				log(location.getErrorString(), ERROR, "The imported file '%s' was not found to be parsable!", file.infileName);
				return;
			}
		}
		catch (AlreadyParsingException e) {
			log(location.getErrorString(), ERROR, "Recursive importing discovered of file %s!", e.getInfileName());
			return;
		}
		if(file.isParsed()) {
			ArrayList<Definition> definitions = file.getPublicRecursiveDefinitions();
			for(Definition def:definitions)
				addImportedDefinition(def, location);
		}
	}
	
	@Override
	protected void onUsingFound(TokenLocation location, InputFile file) {
		try {
			file.parse();
			if(!file.isParsed()) {
				log(location.getErrorString(), ERROR, "The used '%s' file was not found to be parsable!", file.infileName);
				return;
			}
			super.onUsingFound(location, file);
		}
		catch (AlreadyParsingException e) {
			log(location.getErrorString(), ERROR, "Recursive importing discovered of file %s!", e.getInfileName());
			return;
		}
		if(file.isParsed()) {
			ArrayList<Definition> definitions = file.getPublicRecursiveDefinitions();
			for(Definition def:definitions)
				if(def.isPublic())
					addUsedDefinition(def, location);
		}
	}
	
	protected void addUsingDefinition(IncludeDefinition definition) {
		ownDefinitions.add(definition);
	}
	
	private void addImportedDefinition(Definition def, TokenLocation location) {
		if(takenDefinitonNames.contains(def.getName())) {
			log(location.getErrorString(), ERROR, "Definition from '%s' was already defined", def.getStartLocation().getErrorString());
		} else {
			importedDef.add(def);
			takenDefinitonNames.add(def.getName());
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
