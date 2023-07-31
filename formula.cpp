#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

#include "formula.h"
#include "FormulaAST.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

class Formula : public FormulaInterface {
public:
    
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}
    
    Value Evaluate() const override {
        
        try {
            return ast_.Execute();
            
        } catch (const FormulaError& evaluate_error) {
            return evaluate_error;
        }
    }
    
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        
        return out.str();
    }

private:
    FormulaAST ast_;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}