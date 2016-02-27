package me.haved.dafParser.semantic;

import me.haved.dafParser.lexical.TokenLocation;

public class IncludeDefinition implements Definition {
	private TokenLocation location;
	private String file;
	private boolean using;
	
	public IncludeDefinition(TokenLocation location, String file, boolean using) {
		this.location = location;
		this.file = file;
		this.using = using;
	}
	
	public boolean isUsing() {
		return using;
	}
	
	public boolean isImporting() {
		return !using;
	}
	
	public String getFileName() {
		return file;
	}

	@Override
	public String getName() {
		return "Import/Using Definition";
	}

	@Override
	public boolean isPublic() {
		return false;
	}

	@Override
	public TokenLocation getStartLocation() {
		return location;
	}
}
