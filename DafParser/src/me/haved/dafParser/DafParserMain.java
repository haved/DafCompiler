package me.haved.dafParser;

import java.util.Scanner;

public class DafParserMain {
	
	private static boolean DEV = true;
	
	public static void main(String[] args) {
		if(args.length==0) {
			System.out.println("You need to pass arguments. -h for help");
			if(!DEV)
				return;
			System.out.println("You get another chance! Enter your args:");
			Scanner in = new Scanner(System.in);
			String line = in.nextLine();
			in.close();
			args = line.split(" ");
		}
		
		DafParser mainParser = new DafParser();
		mainParser.parseFromLine(args);
	}
}
