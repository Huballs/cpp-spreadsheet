#include <algorithm>
#include <iostream>
#include <optional>

#include "cell.h"
#include "sheet.h"
#include "common.h"

void Sheet::SetCell(Position pos, std::string text) { 
    
    if (!pos.IsValid()) { 
        throw InvalidPositionException("On SetCell");
    }

    auto cell = Table.MakeEmptyCell(pos);

    cell->Set(std::move(text));

    if(Table.IsCircularDependency(cell))
        throw CircularDependencyException("Circular dependency!");

    if (Table(pos))
        Table.DeleteCell(pos);

    Table.SetCellConnections(cell);

    Table.SetCell(cell);
        
}

const CellInterface* Sheet::GetCell(Position pos) const {

    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");
    
    if (!Table.IsPosInside(pos))//  || cells_[pos.row][pos.col]->GetText() == "")
        return nullptr;

    return Table.cells_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {

    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");
    
    if (!Table.IsPosInside(pos))// || cells_[pos.row][pos.col]->GetText() == "")
        return nullptr;

    return Table.cells_[pos.row][pos.col].get();
    
}

void Sheet::ClearCell(Position pos) {
    
    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");
        
    if (Table.IsPosInside(pos)
        && Table(pos)) {
        
        Table(pos)->Clear();

        Table.RemoveCellConnections(pos);
    }
        
}

Size Sheet::GetPrintableSize() const {
    
    Size size;
    
    for (int row = 0; row < static_cast<int>(Table.cells_.size()); ++row) {   

        for (int col = (static_cast<int>(Table.cells_[row].size()) - 1); col >= 0; --col) {
  
            if (Table.cells_[row][col]) {
                
                if (Table.cells_[row][col]->GetText().empty()) {
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
            
            if (col < static_cast<int>(Table.cells_[row].size())
                && Table.cells_[row][col]) {                  

                    std::visit([&output](const auto& obj){output << obj;}, 
                                       Table.cells_[row][col]->GetValue());
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
            
            if (col < static_cast<int>(Table.cells_[row].size())
                && Table.cells_[row][col]) {    

                output << Table.cells_[row][col]->GetText();
            }
        }
        
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}