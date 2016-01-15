package me.haved.dafParser;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Scanner;

public class DafParserMain {
	private static boolean DEV = true;
	
	public static void main(String[] args) {
		if(DEV)
			LogHelper.setMaxLogCount(LogHelper.DEBUG, 200);
		if(args.length==0) {
			LogHelper.println("You need to pass arguments. -h for help");
			if(!DEV)
				return;
			
			LogHelper.println("You get another chance! Enter your args:");
			Scanner in = new Scanner(System.in);
			String line = in.nextLine();
			in.close();
			ArrayList<String> argslist = new ArrayList<>();
			
			argslist.addAll(Arrays.asList(line.trim().split(" ")));
			for(int i = 0; i < argslist.size(); i++) {
				if(argslist.get(i).trim().length()==0) {
					argslist.remove(i);
					i--;
				}
			}
			
			if(args.length < argslist.size())
				args = new String[argslist.size()];
			
			argslist.toArray(args);
		}
		
		DafParser mainParser = new DafParser();
		mainParser.parseFromLine(args);
	}
}
