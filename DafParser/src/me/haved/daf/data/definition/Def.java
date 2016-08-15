package me.haved.daf.data.definition;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.expression.FunctionExpression;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.FunctionType;
import me.haved.daf.data.type.Type;

import static me.haved.daf.LogHelper.*;
;
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

	@Override
	public void codegenDefinitionCpp(PrintWriter cpp, PrintWriter h) {
		if(expression == null) {
			h.print("extern ");
			if(type instanceof FunctionType) {
				h.print(((FunctionType) type).getCppSignature(name));
				h.println(";");
			}
			else
				h.printf("const %s %s;%n", type.codegenCpp(), name);
		}
		else if(expression instanceof FunctionExpression)
			((FunctionExpression) expression).codegenCppAsFunction(cpp, h, name);
		else {
			h.printf("#define %s ", name);
			expression.codegenExpressionCpp(h);
			h.println();
		}
	}

	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		logAssert(!pub);
		logAssert(expression!=null);
		cpp.printf("#define %s ", name);
		expression.codegenExpressionCpp(cpp);
		cpp.println();
	}
}
