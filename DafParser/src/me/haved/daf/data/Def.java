package me.haved.daf.data;

import java.io.PrintStream;

public class Def extends NodeBase implements Definition, Statement {
	
	private String name;
	private Type type;
	private Expression expression;
	private boolean pub;
	
	public Def(String name, Expression expression, boolean pub) {
		
	}
	
	public Def(String name, Type type, Expression expression, boolean pub) {
		this.name = name;
		this.type = type;
		this.expression = expression;
		this.pub = pub;
	}
	
	@Override
	public void print(PrintStream out) {
		out.printf("%sdef %s : %s = %s%n", pub?"pub ":"", name, type.getSignature(), expression.toString());
	}

	@Override
	public boolean isPublic() {
		return pub;
	}

}
