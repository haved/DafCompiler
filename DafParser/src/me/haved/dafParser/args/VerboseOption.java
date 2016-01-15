package me.haved.dafParser.args;

import me.haved.dafParser.DafParser;
import me.haved.dafParser.LogHelper;

public class VerboseOption implements Option {

	@Override
	public boolean canParseOption(String s) {
		return s.equals("-v") | s.equals("--verbose");
	}

	@Override
	public int parseOption(DafParser parser, String[] args, int arg) {
		LogHelper.LOGS = true;
		LogHelper.INFOS = true;
		return 1;
	}

	@Override
	public String getOptionName() {
		return "Verbose Logging";
	}

	@Override
	public String getOptionsForHelpText() {
		return "-v --verbose";
	}

	@Override
	public String getExplanationForHelpText() {
		return "Enables verbose logging.";
	}

}
