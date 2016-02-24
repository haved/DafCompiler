package me.haved.dafParser.semantic;

import me.haved.dafParser.lexical.TokenLocation;

public interface Definition {

	public String getName();
	
	public boolean isPublic();
	
	public TokenLocation getStartLocation();
	
}
