package me.haved.dafParser.semantic;

public class KnownOfClass {
	public String name;
	public boolean interfaceDef;
	public boolean publicClass;
	
	public KnownOfClass(String name, boolean interfaceDef, boolean publicClass) {
		this.name = name;
		this.interfaceDef = interfaceDef;
		this.publicClass = publicClass;
	}
}
