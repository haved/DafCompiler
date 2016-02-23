package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;

import me.haved.dafParser.args.HelpTextOption;
import me.haved.dafParser.args.Option;
import me.haved.dafParser.args.SubfolderOutputOption;
import me.haved.dafParser.args.SummarizeOption;
import me.haved.dafParser.args.VerboseOption;

import static me.haved.dafParser.LogHelper.*;

public class DafParser {
	private ArrayList<Option> options;

	public boolean subfolderOutput = false;
	
	public DafParser() {
		options = new ArrayList<>();
		options.add(new VerboseOption());
		options.add(new SummarizeOption());
		options.add(new SubfolderOutputOption());
		options.add(new HelpTextOption());
	}
	
	public void parseFromLine(String[] args) {
		String inputFile=null;
		String outputDir=null;
		
		int lineSteps = 0;
		
		int i = 0;
		boolean parsed;
		while(i < args.length) {
			parsed = false;
			for(Option option:options) {
				if(option.canParseOption(args[i])) {
					parsed = true;
					int returned = option.parseOption(this, args, i);
					if(returned==0) {
						log(MESSAGE, "Parsing stopped by option '%s', (%s)", option.getOptionName(), args[i]);
						return;
					}
					i+=returned;
					break;
				}
			}
			if(!parsed && args[i].startsWith("-")) {
				log(FATAL_ERROR, "%s was not recogniced as an option!", args[i]);
				return;
			}
			if(!parsed) {
				if(lineSteps == 0)
					inputFile = args[i];
				else if(lineSteps==1)
					outputDir = args[i];
				lineSteps++;
				i++;
			}
		}
		if(i>args.length) {
			log(FATAL_ERROR, "Oh shit, man. An option said it used two args, but there were not 'nuff");
		}
		else if(lineSteps<2) {
			log(FATAL_ERROR, "Expecting the argument: %s.", lineSteps==0?"inputFile":"outputDir");
		}
		else if(lineSteps!=2)
			log(FATAL_ERROR, "Too may arguments were supplied!");
		
		executeParsing(inputFile, outputDir);
	}
	
	public void executeParsing(String inputFilePath, String outputDirPath){
		log(INFO, "Thinking of parsing file '%s' and storing .cpp and .h files in '%s'", inputFilePath, outputDirPath);
		
		try {
			File inputFile = new File(inputFilePath);
			if(inputFile.exists()==false) {
				log(FATAL_ERROR, "The input file '%s' does not exist!", inputFile.getAbsolutePath());
			} else if(inputFile.isFile()==false) {
				log(FATAL_ERROR, "The input '%s' is not a file!", inputFile.getAbsolutePath());
			}
			File outputDir = new File(outputDirPath);
			if(outputDir.exists()==false) {
				log(FATAL_ERROR, "The output directory '%s' does not exist!", outputDir.getAbsolutePath());
			} else if(outputDir.isDirectory()==false) {
				log(FATAL_ERROR, "The output directory '%s' is not a directory!", outputDir.getAbsolutePath());
			}
			
			//ParsedInputFile parsedInputFile = ParsedInputFile.makeInputFileInstance(inputFile, inputFilePath, true); //file name, really
			
			//parsedInputFile.parse();
			
			String outputFilesPath = outputDir.getAbsolutePath() + "/" + (subfolderOutput?inputFilePath:inputFile.getName());
			outputFilesPath=outputFilesPath.substring(0, outputFilesPath.lastIndexOf('.'));
			
			File cppFile =    new File(outputFilesPath+".cpp");
			File headerFile = new File(outputFilesPath+".h");
			
			if(subfolderOutput)
				cppFile.getParentFile().mkdirs();
			
			log(INFO, "In diretctory '%s', making '%s' and '%s'", cppFile.getParent(), cppFile.getName(), headerFile.getName());
			//parsedInputFile.writeToCppAndHeader(cppFile, headerFile);
		}
		catch(Exception e) {
			e.printStackTrace(out);
		}
	}
	
	public void printHelpText() {
		println("The help text for the daf parser.");
		println("daf <inputFile> <outputDir> [options]");
		ArrayList<String> optionNames = new ArrayList<String>();
		ArrayList<String> optionText  = new ArrayList<String>();
		
		int maxLength = 0;
		for(Option option : options) {
			String optionName = option.getOptionsForHelpText();
			maxLength = Math.max(maxLength, optionName.length());
			
			optionNames.add(optionName);
			optionText.add(option.getExplanationForHelpText());
		}
		
		for(int i = 0; i < optionNames.size(); i++) {
			String name = optionNames.get(i);
			println("%1$s %2$"+(maxLength-name.length()+1)+"s       %3$s", name, "", optionText.get(i));
		}
	}
}
