package me.haved.daf;

import static me.haved.daf.LogHelper.*;

import java.util.Scanner;

public class MainDafParser {
	public static void main(String[] args) {
		log(SUPER_DEBUG, "Welcome to the daf parser!");
		log(SUGGESTION, "Hit enter to exit!");
		try (Scanner s = new Scanner(System.in)) {
			s.nextLine();
		}
		if(Math.random()<0.5f)
			log(FATAL_ERROR, "Quick eject!");
		LogHelper.printSummary(0);
	}
}
