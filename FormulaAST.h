#pragma once

#include "FormulaLexer.h"
#include "common.h"

#include <vector>
#include <functional>
#include <stdexcept>

namespace ASTImpl {
    class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute() const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::vector<Position>& GetCells();

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::vector<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);