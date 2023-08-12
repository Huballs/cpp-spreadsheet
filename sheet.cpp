#include <algorithm>
#include <iostream>
#include <optional>

#include "cell.h"
#include "sheet.h"
#include "common.h"

// --- Table ---

CellPtr Table::operator()(Position pos){
    ResizeToFit(pos);
    return cells_[pos.row][pos.col];
}

inline void Table::SetCell(CellPtr cell){

    Position pos = cell->GetPosition();

    ResizeToFit(pos);
    cells_[pos.row][pos.col] = cell;
}

inline void Table::DeleteCell(Position pos){
    if(!IsPosInside(pos)){
        throw InvalidPositionException("On Table::DeleteCell");
    }
    RemoveCellConnections(pos);
    cells_[pos.row][pos.col].reset();
}

inline void Table::RemoveCellConnections(Position pos){

    for(const auto& ref_pos : pos_to_refs[pos]){
        cell_to_deps[ref_pos].erase(pos);
    }
    
    pos_to_refs.erase(pos);
}

inline void Table::ResizeToFit(Position pos){
    cells_.resize(std::max(pos.row + 1, static_cast<int>(cells_.size())));
    cells_[pos.row].resize(std::max(pos.col + 1, static_cast<int>(cells_[pos.row].size())));
}

inline bool Table::IsPosInside(Position pos) const {
    if  (  pos.row < static_cast<int>(cells_.size()) 
        && pos.col < static_cast<int>(cells_[pos.row].size())){

        return true;
    }
    return false;
}


// --- Sheet --

CellPtr Sheet::MakeEmptyCell(Position pos){
    return std::make_shared<Cell>(*this, pos);
}


void Sheet::SetCell(Position pos, std::string text) { 
    
    if (!pos.IsValid()) { 
        throw InvalidPositionException("On SetCell");
    }

    auto cell = MakeEmptyCell(pos);

    cell->Set(std::move(text));

    if(IsCircularDependency(cell))
        throw CircularDependencyException("circular dependency detected");

    if (table_(pos))
        table_.DeleteCell(pos);

    SetCellConnections(cell);

    table_.SetCell(cell);
        
}

const CellInterface* Sheet::GetCell(Position pos) const {

    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");
    
    if (!table_.IsPosInside(pos))//  || cells_[pos.row][pos.col]->GetText() == "")
        return nullptr;

    return table_.cells_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {

    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");
    
    if (!table_.IsPosInside(pos))// || cells_[pos.row][pos.col]->GetText() == "")
        return nullptr;

    return table_.cells_[pos.row][pos.col].get();
    
}

void Sheet::ClearCell(Position pos) {
    
    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");
        
    if (table_.IsPosInside(pos)
        && table_(pos)) {
        
        table_(pos)->Clear();

        table_.RemoveCellConnections(pos);
    }
        
}

Size Sheet::GetPrintableSize() const {
    
    Size size;
    
    for (int row = 0; row < static_cast<int>(table_.cells_.size()); ++row) {   

        for (int col = (static_cast<int>(table_.cells_[row].size()) - 1); col >= 0; --col) {
  
            if (table_.cells_[row][col]) {
                
                if (table_.cells_[row][col]->GetText().empty()) {
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
            
            if (col < static_cast<int>(table_.cells_[row].size())
                && table_.cells_[row][col]) {                  

                    std::visit([&output](const auto& obj){output << obj;}, 
                                       table_.cells_[row][col]->GetValue());
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
            
            if (col < static_cast<int>(table_.cells_[row].size())
                && table_.cells_[row][col]) {    

                output << table_.cells_[row][col]->GetText();
            }
        }
        
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}