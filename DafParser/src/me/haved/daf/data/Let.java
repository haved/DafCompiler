package me.haved.daf.data;

import java.io.PrintStream;

import me.haved.daf.data.expression.Expression;

public class Let extends NodeBase implements Definition, Statement {

	private String name;
	private Type type;
	private Expression expression;
	private boolean pub;
	
	public Let(String name, Type type, Expression expression, boolean pub) {
		this.name = name;
		this.type = type;
		this.expression = expression;
		this.pub = pub;
	}
	
	public Let(String name, Type type, Expression expression) {
		this(name, type, expression, false);
	}
	
	@Override
	public void print(PrintStream out) {
		if(expression!=null)
			out.printf("%slet %s : %s = %s;%n", pub?"pub ":"", name, type==null?"null":type.getSignature(), expression.toString());
		else
			out.printf("%slet %s : %s;%n", pub?"pub ":"", name, type==null?"null":type.getSignature());
	}

	@Override
	public boolean isPublic() {
		return pub;
	}
}
