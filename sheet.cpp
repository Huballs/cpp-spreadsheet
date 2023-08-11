#include <algorithm>
#include <iostream>
#include <optional>

#include "cell.h"
#include "sheet.h"
#include "common.h"

inline bool Sheet::IsPositionInsideSheet(const Position& pos) const{

    if  (  pos.row < static_cast<int>(cells_.size()) 
        && pos.col < static_cast<int>(cells_[pos.row].size())){

        return true;
    }

    return false;
}

void Sheet::MakeEmptyCell(Position pos){
    cells_[pos.row][pos.col] = MakeEmptyCell(); 
}

CellPtr Sheet::MakeEmptyCell(){
    return std::make_shared<Cell>(*this);
}

void Sheet::DeleteCell(CellPtr& cell, Position pos){

    if(!(pos == Position::NONE)){
        pos_to_refs.erase(pos);

        auto it = cell_to_deps.find(cell);

        if((it != cell_to_deps.end()) 
        && (it->second.empty())){
            cell_to_deps.erase(it);
        }
    }

    cell.reset();
}

void Sheet::SetCell(Position pos, std::string text) { 
    
    if (pos.IsValid()) { 

        auto cell = MakeEmptyCell();

        cell->Set(std::move(text));

        SetCellRefs(cell, pos);
        SetCellDependants(cell, pos);

        cells_.resize(std::max(pos.row + 1, static_cast<int>(cells_.size())));
        cells_[pos.row].resize(std::max(pos.col + 1, static_cast<int>(cells_[pos.row].size())));

        if (cells_[pos.row][pos.col])
            DeleteCell(cells_[pos.row][pos.col], pos);

        cells_[pos.row][pos.col] = cell;
        
    } else {
        throw InvalidPositionException("On SetCell");
    }    
}

const CellInterface* Sheet::GetCell(Position pos) const {

    if (pos.IsValid()) {
        
        if (IsPositionInsideSheet(pos)
            && cells_[pos.row][pos.col].get()->GetText() != "") {
      
            return cells_[pos.row][pos.col].get();
                     
        } 
        
        return nullptr;
        
    } else {
        throw InvalidPositionException("On GetCell");
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    
    if (pos.IsValid()) {
        
        if (IsPositionInsideSheet(pos)
            && cells_[pos.row][pos.col].get()->GetText() != "") {
      
            return cells_[pos.row][pos.col].get();
                     
        } 
        
        return nullptr;
        
    } else {
        throw InvalidPositionException("On GetCell");
    }
}

void Sheet::ClearCell(Position pos) {
    
    if (pos.IsValid()) {
        
        if (IsPositionInsideSheet(pos)
            && cells_[pos.row][pos.col]) {
            
            cells_[pos.row][pos.col]->Clear();
        }
        
    } else {    
        throw InvalidPositionException("On ClearCell");
    } 
}

Size Sheet::GetPrintableSize() const {
    
    Size size;
    
    for (int row = 0; row < static_cast<int>(cells_.size()); ++row) {   

        for (int col = (static_cast<int>(cells_[row].size()) - 1); col >= 0; --col) {
  
            if (cells_[row][col]) {
                
                if (cells_[row][col]->GetText().empty()) {
                    continue;
                } 

                size.rows = std::max(size.rows, row + 1);
                size.cols = std::max(size.cols, col + 1);
                break;
            }
        }
    }
    
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    
    for (int row = 0; row < GetPrintableSize().rows; ++row) {     
        for (int col = 0; col < GetPrintableSize().cols; ++col) {
            
            if (col > 0) 
                output << '\t';
            
            if (col < static_cast<int>(cells_[row].size())
                && cells_[row][col]) {                  

                    std::visit([&output](const auto& obj){output << obj;}, 
                                       cells_[row][col]->GetValue());
            }
        }
        
        output << '\n';
    }
}
            
void Sheet::PrintTexts(std::ostream& output) const {
   
    for (int row = 0; row < GetPrintableSize().rows; ++row) {        
        for (int col = 0; col < GetPrintableSize().cols; ++col) {
            
            if (col > 0) 
                output << '\t';
            
            if (col < static_cast<int>(cells_[row].size())
                && cells_[row][col]) {    

                output << cells_[row][col]->GetText();
            }
        }
        
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}