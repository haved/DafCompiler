package me.haved.daf.data.expression;

import me.haved.daf.data.Node;
import me.haved.daf.data.type.Type;

public interface Expression extends Node {
	public boolean isTypeSet();
	public boolean tryEvaluatingType();
	public Type getType();
}
