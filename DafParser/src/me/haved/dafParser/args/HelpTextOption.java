package me.haved.dafParser.args;

import me.haved.dafParser.DafParser;

public class HelpTextOption implements Option {
	
	@Override
	public boolean canParseOption(String s) {
		return s.equals("-h") | s.equals("--help") | s.equals("?") | s.equals("-?");
	}

	@Override
	public int parseOption(DafParser parser, String[] args, int arg) {
		parser.printHelpText();
		return 0;
	}

	@Override
	public String getOptionName() {
		return "Help";
	}
	
	@Override
	public String getOptionsForHelpText() {
		return "-h --help ? -?";
	}

	@Override
	public String getExplanationForHelpText() {
		return "Prints this help message.";
	}

}
