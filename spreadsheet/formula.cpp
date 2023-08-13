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

 
FormulaError::FormulaError(Category category) {
    category_ = category;
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const{
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    
    switch (category_) {
            
        case Category::Ref:
            return "#REF!";
            
        case Category::Value:
            return "#VALUE!";
            
        case Category::Div0:
            return "#DIV/0!";
    }
    return "";
}

class Formula : public FormulaInterface {
public:
    
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}
    
    Value Evaluate(const SheetInterface& sheet) const override {
 
        try {
            std::function<double(Position)> args = [&sheet](const Position pos)->double {
           
            if (!pos.IsValid()) 
                throw FormulaError(FormulaError::Category::Ref);

            const auto* cell = sheet.GetCell(pos);
            if (!cell) 
                return 0.0;

            if (std::holds_alternative<double>(cell->GetValue())) {
                return std::get<double>(cell->GetValue());
                
            } else 
            if (std::holds_alternative<std::string>(cell->GetValue())) {
                
                auto str_value = std::get<std::string>(cell->GetValue());

                if (str_value == "") 
                    return 0.0;

                std::istringstream input(str_value);
                double num = 0.0;

                if (input >> num && input.eof()) {
                    return num;
                } else {
                    throw FormulaError(FormulaError::Category::Value);
                }
                
            } else {
                throw FormulaError(std::get<FormulaError>(cell->GetValue()));
            }
                
            };
            
            return ast_.Execute(args);
            
        } catch (const FormulaError& evaluate_error) {
            return evaluate_error;
        }
    }
    
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        for (const auto& cell : ast_.GetCells()) {
            
            if (!cell.IsValid()) 
                continue;
            if (cells.size() == 0 
            || (cells.size() && !(cell == cells.back())))
                cells.push_back(cell);

        }
        return cells;
    }

private:
    FormulaAST ast_;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}