#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <limits>

#include "cell.h"

// --- Cell ---

void Cell::Set(std::string text) {
    
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        
    } else if (text.size() >= 2 && text.at(0) == FORMULA_SIGN) {
        impl_ = std::move(std::make_unique<FormulaImpl>(std::move(text), sheet_));
        
    } else {
        impl_ = std::move(std::make_unique<TextImpl>(std::move(text)));
    }
    InvalidateCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::InvalidateCache(){
    impl_->InvalidateCache();
}   

// --- Cell::EmptyImpl ---

Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

// --- Cell::TextImpl ---

Cell::TextImpl::TextImpl(std::string text) 
    : text_(std::move(text)) {}

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

// --- Cell::FormulaImpl ---

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) 
    : formula_ptr_(ParseFormula(text.substr(1))), sheet_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {   

    if(isCacheValid){
        return cache_;
    }          
    auto result = formula_ptr_->Evaluate(sheet_);
    
    if (std::holds_alternative<double>(result)) {

        double number = std::get<double>(result);
        
        if(number == std::numeric_limits<double>::infinity()
        || number == -std::numeric_limits<double>::infinity())
            cache_ = FormulaError(FormulaError::Category::Div0);
        else
            cache_ = number;
    } else {
        cache_ = std::get<FormulaError>(result);
    }

    isCacheValid = true;

    return cache_;  
         
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const{
    return formula_ptr_->GetReferencedCells();
}

Position Cell::GetPosition() const{
    return position_;
}

void Cell::FormulaImpl::InvalidateCache(){
    isCacheValid = false;
}
