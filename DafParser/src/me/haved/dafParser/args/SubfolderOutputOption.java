package me.haved.dafParser.args;

import me.haved.dafParser.DafParser;

public class SubfolderOutputOption implements Option {

	@Override
	public boolean canParseOption(String s) {
		return s.equals("-sfo") | s.equals("--subfolder-output");
	}

	@Override
	public int parseOption(DafParser parser, String[] args, int arg) {
		parser.subfolderOutput = true;
		return 1;
	}

	@Override
	public String getOptionName() {
		return "Subfolder output";
	}

	@Override
	public String getOptionsForHelpText() {
		return "-sfo --subfolder-output";
	}

	@Override
	public String getExplanationForHelpText() {
		return "Puts output in subfolders if input is in subfolders";
	}
}
