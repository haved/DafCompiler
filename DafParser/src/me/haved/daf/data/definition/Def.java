package me.haved.daf.data.definition;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;

public class Def extends NodeBase implements Definition, Statement {
	
	private String name;
	private Type type;
	private Expression expression;
	private boolean pub;
	
	public Def(String name, Type type, Expression expression, boolean pub) {
		this.name = name;
		this.type = type;
		this.expression = expression;
		this.pub = pub;
	}
	
	@Override
	public String getSignature() {
		if(type == null)
			return String.format("%sdef %s:=%s;", pub?"pub ":"",name,expression);
		else
			return String.format("%sdef %s:%s=%s;", pub?"pub ":"",name,type.getSignature(),expression);
	}

	@Override
	public boolean isPublic() {
		return pub;
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
}
