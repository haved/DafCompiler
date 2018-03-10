#pragma once
#include "parsing/ast/Type.hpp"
#include "CodegenLLVMForward.hpp"

enum class ConcreteTypeKind {
	FUNCTION=0,
	POINTER,
	PRIMITIVE,
	VOID,
	END_OF_ENUM
};

void printConcreteTypeKind(ConcreteTypeKind kind, std::ostream& out);

class ConcreteType;
struct ExprTypeInfo;
enum class ValueKind;
struct CTypeKindFilter { //Yes this is maybe a bit overkill
private: CTypeKindFilter(int filter);
public:
	int filter;
	static CTypeKindFilter allowingNothing();
	static CTypeKindFilter allowingEverything();
	CTypeKindFilter alsoAllowing(ConcreteTypeKind kind) const;
	CTypeKindFilter butDisallowing(ConcreteTypeKind kind) const;
	CTypeKindFilter ored(const CTypeKindFilter& other) const;
	CTypeKindFilter unioned(const CTypeKindFilter& other) const;
	CTypeKindFilter inversed() const;
	bool allows(ConcreteTypeKind kind) const;
	bool allows(ConcreteType* type) const;
	bool allowsAndHasValueKind(const ExprTypeInfo& attempt, ValueKind requiredKind) const;
	void printAllPosibilities(std::ostream& out) const;
};

enum class CastPossible;
struct EvaluatedExpression;
class ConcreteType {
public:
	ConcreteType()=default;
	virtual ~ConcreteType()=default;
	virtual void printSignature()=0;
	virtual ConcreteTypeKind getConcreteTypeKind() const =0;

	virtual bool hasSize()=0;
	//Assumed to be a different type, but the valueKind can go the "wrong" direction
	virtual CastPossible canConvertTo(ValueKind fromKind, ExprTypeInfo& B)=0;
	//Same here, so won't ask for a possible conversion to same ConcreteTypeKind
	virtual optional<ExprTypeInfo> getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights)=0;
	virtual optional<EvaluatedExpression> codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target)=0;
	virtual llvm::Type* codegenType(CodegenLLVM& codegen)=0;
};

struct ExprTypeInfo;

class ConcretePointerType : public ConcreteType {
private:
	bool m_mut;
	ConcreteType* m_target;
	ConcretePointerType(bool mut, ConcreteType* target);
public:
	ConcretePointerType(const ConcretePointerType& other)=delete;
	ConcretePointerType& operator=(const ConcretePointerType& other)=delete;
	virtual void printSignature() override;
	virtual ConcreteTypeKind getConcreteTypeKind() const override;
    virtual bool hasSize() override;
	ExprTypeInfo getDerefResultExprTypeInfo();

	virtual CastPossible canConvertTo(ValueKind fromKind, ExprTypeInfo& target) override;
	virtual optional<ExprTypeInfo> getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) override;
	virtual optional<EvaluatedExpression> codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) override;

	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;

	static ConcretePointerType* toConcreteType(bool mut, ConcreteType* type);
};

enum class Signed {
	Yes, No, NA
};

class PrimitiveType : public ConcreteType {
private:
	LiteralKind m_literalKind;
	TokenType m_token;
	bool m_floatingPoint;
	bool m_signed;
	int m_bitCount;
public:
	PrimitiveType(LiteralKind literalKind, TokenType token, bool floatingPoint, Signed isSigned, int bitCount);
	virtual void printSignature() override;
	virtual ConcreteTypeKind getConcreteTypeKind() const override { return ConcreteTypeKind::PRIMITIVE; }

	LiteralKind getLiteralKind();
	TokenType getTokenType();
	bool isFloatingPoint();
	bool isSigned();
	int getBitCount();

	virtual bool hasSize() override;

	virtual CastPossible canConvertTo(ValueKind fromKind, ExprTypeInfo& target) override;
	virtual optional<ExprTypeInfo> getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) override;
	virtual optional<EvaluatedExpression> codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) override;

	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;
};

PrimitiveType* tokenTypeToPrimitiveType(TokenType type);
PrimitiveType* literalKindToPrimitiveType(LiteralKind kind);
PrimitiveType* castToPrimitiveType(ConcreteType* type);

class VoidType : public ConcreteType {
public:
	virtual void printSignature() override;
    virtual ConcreteTypeKind getConcreteTypeKind() const override { return ConcreteTypeKind::VOID; }

	virtual bool hasSize() override;

	virtual CastPossible canConvertTo(ValueKind fromKind, ExprTypeInfo& target) override;
	virtual optional<ExprTypeInfo> getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) override;
	virtual optional<EvaluatedExpression> codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) override;

	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;
};

VoidType* getVoidType();

//Function type defined in FunctionSignature.hpp
