package me.haved.dafParser;

import java.io.File;
import java.util.ArrayList;

import me.haved.dafParser.args.HelpTextOption;
import me.haved.dafParser.args.Option;
import me.haved.dafParser.args.VerboseOption;

public class DafParser {
	private ArrayList<Option> options;
	
	public DafParser() {
		options = new ArrayList<>();
		options.add(new VerboseOption());
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
						LogHelper.Info("Parsing stopped by option '%s', (%s)%n", option.getOptionName(), args[i]);
						return;
					}
					i+=returned;
				}
			}
			if(!parsed && args[i].startsWith("-")) {
				LogHelper.Error("%s was not recogniced as an option!%n", args[i]);
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
			System.out.println("Oh shit, man. An option said it used two args, but there were not 'nuff");
			return;
		}
		if(lineSteps!=2) {
			System.out.printf("Too %s arguments were given. -h for help%n", lineSteps<2?"few":"many");
			return;
		}
		
		executeParsing(inputFile, outputDir);
	}
	
	public void executeParsing(String inputFilePath, String outputDirPath){
		LogHelper.Info("Parsing file %s and storing .cpp and .h files in %s%n", inputFilePath, outputDirPath);
		
		try {
			File inputFile = new File(inputFilePath);
			if(!inputFile.exists()) {
				LogHelper.Error("The input file '%s' does not exist.%n", inputFile.getName());
			}
		}
		catch(Exception e) {
			LogHelper.Println("Aborting due to %d previous error(s)", );
		}
	}
	
	public void printHelpText() {
		System.out.println("The help text for the daf parser.");
		System.out.println("daf inputFile outputDir");
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
			System.out.printf("%1$s : %2$"+(maxLength-name.length()+1)+"s       %3$s%n", name, "", optionText.get(i));
		}
	}
}
