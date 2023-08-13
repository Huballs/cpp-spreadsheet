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

    InvalidateCacheOfDependants(pos);
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
        throw InvalidPositionException("On ClearCell");
        
    if (table_.IsPosInside(pos)
        && table_(pos)) {
        
        table_.DeleteCell(pos);
        InvalidateCacheOfDependants(pos);
    }
        
}

void Sheet::SetCellConnections(CellPtr cell){
    SetCellRefs(cell);
    SetCellDependants(cell);
}
 
void Sheet::InvalidateCacheOfDependants(Position pos){

    std::function<void (Position)> 
        go_up = [&](Position pos){
            for(const auto& dep_pos : table_.cell_to_deps[pos]){
                table_(dep_pos)->InvalidateCache();
                if(table_.cell_to_deps.count(dep_pos))
                    go_up(dep_pos);
            }        
        };

    if(table_.cell_to_deps.count(pos))
        go_up(pos);
    
}

void Sheet::SetCellRefs(CellPtr cell){

    for(const auto ref_pos : cell->GetReferencedCells()){

        auto ref_cell = table_(ref_pos);

        if(!ref_cell){
            ref_cell = MakeEmptyCell(ref_pos);
            table_.SetCell(ref_cell);
        }

        table_.pos_to_refs[cell->GetPosition()].insert(ref_pos);
    }

}

void Sheet::SetCellDependants(CellPtr cell){
    for(const auto ref_pos : cell->GetReferencedCells()){
        table_.cell_to_deps[ref_pos].insert(cell->GetPosition());
    }
}

bool Sheet::IsCircularDependency(CellPtr cell){

    std::function<bool (Position, const std::vector<Position>&)> 
        go_up = [&](Position pos, const std::vector<Position>& pos_to_find){

        auto it = table_.cell_to_deps.find(pos);

        if(it != table_.cell_to_deps.end()){

            if(it->second.empty()){
                table_.cell_to_deps.erase(it);
                return false;
            }

            for(const auto dep_pos : it->second){

                auto predicate = [&dep_pos](const Position& lhs){ return lhs == dep_pos;};

                if (std::any_of(pos_to_find.begin(), pos_to_find.end(), predicate))
                    return true;
                return go_up(dep_pos, pos_to_find);
            }
        }
        return false;
    };

    for(const auto& ref_pos : cell->GetReferencedCells()){

        if(ref_pos == cell->GetPosition())
            return true;
    }

    return go_up(cell->GetPosition(), cell->GetReferencedCells());

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
