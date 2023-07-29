#include <cassert>
#include <iostream>
#include <string>
#include <optional>

#include "cell.h"

void Cell::Set(std::string text) {
    
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        
    } else if (text.size() >= 2 && text.at(0) == FORMULA_SIGN) {
        impl_ = std::move(std::make_unique<FormulaImpl>(std::move(text)));
        
    } else {
        impl_ = std::move(std::make_unique<TextImpl>(std::move(text)));
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {return impl_->GetValue();}
std::string Cell::GetText() const {return impl_->GetText();}

Cell::Value Cell::EmptyImpl::GetValue() const {return "";}
std::string Cell::EmptyImpl::GetText() const {return "";}

Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw std::logic_error("Empty text");
        
    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);
        
    } else {
        return text_;    
    }      
}

std::string Cell::TextImpl::GetText() const {
    return text_;
    }

Cell::FormulaImpl::FormulaImpl(std::string text) 
    : formula_ptr_(ParseFormula(text.substr(1))) {}

Cell::Value Cell::FormulaImpl::GetValue() const {             
    auto helper = formula_ptr_->Evaluate();
    
    if (std::holds_alternative<double>(helper)) {
        return std::get<double>(helper);
    } else {
        return std::get<FormulaError>(helper);  
    }       
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
    }