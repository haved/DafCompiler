#pragma once

#include "parsing/semantic/Concretable.hpp"
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Namespace.hpp"
#include "parsing/semantic/NamedDefinitionMap.hpp"
#include "CodegenLLVMForward.hpp"
#include <memory>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <map>

using std::unique_ptr;
using boost::optional;

enum class DefinitionKind {
	DEF,
	LET,
	TYPEDEF,
	NAMEDEF,
	WITH
};

class Definition : public Concretable {
protected:
	bool m_pub;
	TextRange m_range;
	Definition(bool pub, const TextRange& range);
public:
	virtual ~Definition();
	inline const TextRange& getRange() { return m_range; }
	inline bool isPublic() {return m_pub;}

	virtual void addToMap(NamedDefinitionMap& map)=0;
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override=0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override=0;

	//@Optimize: lots of dead virtual calls (Though O(n))
	virtual void globalCodegen(CodegenLLVM& codegen);
	virtual void localCodegen(CodegenLLVM& codegen);

	virtual void printSignature()override=0; //Children print 'pub ' if needed
	virtual DefinitionKind getDefinitionKind() const =0;
};

class Def : public Definition {
private:
	std::string m_name;
	unique_ptr<FunctionExpression> m_functionExpression;
public:
	Def(bool pub, std::string&& name, unique_ptr<FunctionExpression>&& expression, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	bool allowsImplicitAccess();
	FunctionExpression* getFunctionExpression();
	const ExprTypeInfo& getFunctionExpressionTypeInfo();

	virtual void globalCodegen(CodegenLLVM& codegen) override;
	virtual void localCodegen(CodegenLLVM& codegen) override;

	optional<EvaluatedExpression> functionAccessCodegen(CodegenLLVM& codegen);

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::DEF; }
};

class Let : public Definition {
private:
	bool m_mut;
	std::string m_name;
	TypeReference m_givenType;
	unique_ptr<Expression> m_expression;

	ExprTypeInfo m_typeInfo;

	FunctionExpression* m_definingFunction;

	llvm::Value* m_space;
	bool m_stealSpaceFromTarget; //Used for reference function parameters
public:
	Let(bool pub, bool mut, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange& range, bool stealSpaceFromTarget=false);
	Let(const Let& other) = delete;
	Let(Let&& other) = default;
	Let& operator=(const Let& other) = delete;
	Let& operator=(Let&& other) = default;

	virtual void addToMap(NamedDefinitionMap& map) override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;
	const ExprTypeInfo& getTypeInfo() const;
    optional<FunctionExpression*> getDefiningFunction();

	virtual void globalCodegen(CodegenLLVM& codegen) override;
	virtual void localCodegen(CodegenLLVM& codegen) override;

	optional<EvaluatedExpression> accessCodegen(CodegenLLVM& codegen);

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::LET; }
};

//WithDefinition is in With.hpp

class TypedefDefinition : public Definition {
	std::string m_name;
	TypeReference m_type;
public:
	TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;
   	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;
	ConcreteType* getConcreteType();

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::TYPEDEF; }
};

class ConcreteNameScope;
enum class NameScopeExpressionKind;

class NameScopeExpression : public Concretable {
private:
	int m_blockLevel=0;
protected:
	TextRange m_range;
public:
	NameScopeExpression(const TextRange& range);
	virtual ~NameScopeExpression();
	inline const TextRange& getRange() { return m_range; }
	void setBlockLevel(int blockLevel);
	int getBlockLevel() const;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override=0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual ConcreteNameScope* getConcreteNameScope()=0;

	virtual void codegen(CodegenLLVM& codegen) {(void) codegen; std::cout << "TODO: NameScopeExpression codegen" << std::endl; }

	virtual void printSignature()override =0;
	virtual NameScopeExpressionKind getNameScopeExpressionKind()=0;
};

class NamedefDefinition : public Definition {
private:
	std::string m_name;
	unique_ptr<NameScopeExpression> m_value;
public:
	NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range);
	virtual void addToMap(NamedDefinitionMap& map) override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	ConcreteNameScope* getConcreteNameScope();

	virtual void globalCodegen(CodegenLLVM& codegen) override;
	virtual void localCodegen(CodegenLLVM& codegen) override;

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::NAMEDEF; }
};

std::ostream& printDefinitionKindName(DefinitionKind kind, std::ostream& out);
