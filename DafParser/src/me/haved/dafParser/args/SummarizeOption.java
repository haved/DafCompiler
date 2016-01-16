package me.haved.dafParser.args;

import me.haved.dafParser.DafParser;
import me.haved.dafParser.LogHelper;

public class SummarizeOption implements Option {

	@Override
	public boolean canParseOption(String s) {
		return s.equals("-s") | s.equals("--summarize");
	}

	@Override
	public int parseOption(DafParser parser, String[] args, int arg) {
		LogHelper.setToSummarize(true);
		return 1;
	}

	@Override
	public String getOptionName() {
		return "Enable Summarization";
	}

	@Override
	public String getOptionsForHelpText() {
		return "-s --summarize";
	}

	@Override
	public String getExplanationForHelpText() {
		return "Give a log summarization after execution";
	}
}
