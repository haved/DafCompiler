package me.haved.daf.args;

public class HelpOption implements CommandOption {
	
	private HelpCall call;
	
	public HelpOption(HelpCall call) {
		this.call = call;
	}
	
	@Override
	public int parseOption(String[] args, int pos) {
		if(args[pos].equals("-h") | args[pos].equals("--help")) {
			call.help();
			return STOP_PARSING;
		}
		return WRONG_PARSE;
	}

	@Override
	public String getName() {
		return "-h --help";
	}

	@Override
	public String getDescription() {
		return "Prints this help message";
	}

	public static interface HelpCall {
		void help();
	}
}
