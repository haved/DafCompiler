package me.haved.daf.data;

public class PointerType extends NodeBase implements Type {
	
	private Type base;
	
	public PointerType(Type base) {
		this.base = base;
	}
}
