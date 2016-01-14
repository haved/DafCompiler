package me.haved.dafParser;

import java.util.ArrayList;

import me.haved.dafParser.args.HelpTextOption;
import me.haved.dafParser.args.Option;

public class DafParser {
	private ArrayList<Option> options;
	
	public DafParser() {
		options = new ArrayList<>();
		options.add(new HelpTextOption());
	}
	
	public void parseFromLine(String[] args) {
		int i = 0;
		while(i < args.length) {
			for(Option option:options) {
				if(option.canParseOption(args[i])) {
					int returned = option.parseOption(this, args, i);
					if(returned==0) {
						System.out.printf("Parsing stopped by option '%s', (%s)%n", option.getOptionName(), args[i]);
						return;
					}
					i+=returned;
				}
			}
		}
		if(i>args.length) {
			System.out.println("Oh shit, man. An option said it used two args, but there were not 'nuff");
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
			System.out.printf("%s%s%n", String.format("%1s : %2$"+(maxLength-name.length()+1)+"s", name, ""), optionText.get(i));
		}
	}
}
