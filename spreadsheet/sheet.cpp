#include <algorithm>
#include <iostream>
#include <optional>

#include "cell.h"
#include "sheet.h"
#include "common.h"

// --- Table ---

CellPtr Table::operator()(Position pos){
    auto it = cells_.find(pos);
    if(it != cells_.end())
        return it->second;
    return nullptr;
}

const CellPtr Table::operator()(Position pos) const {
    auto it = cells_.find(pos);
    if(it != cells_.end())
        return it->second;
    return nullptr;
}

inline void Table::SetCell(CellPtr cell){

    Position pos = cell->GetPosition();

    cells_[pos] = cell;
}

inline void Table::DeleteCell(Position pos){

    RemoveCellConnections(pos);

    cells_.erase(pos);
}

inline void Table::RemoveCellConnections(Position pos){

    for(const auto& ref_pos : pos_to_refs[pos]){
        cell_to_deps[ref_pos].erase(pos);
    }
    
    pos_to_refs.erase(pos);
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

    return table_(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {

    if(!pos.IsValid())
        throw InvalidPositionException("On GetCell");


    return table_(pos).get();
    
}

void Sheet::ClearCell(Position pos) {
    
    if(!pos.IsValid())
        throw InvalidPositionException("On ClearCell");
        
    table_.DeleteCell(pos);
    InvalidateCacheOfDependants(pos);
     
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
    
    Size result{ 0, 0 };
    
    for (auto it = table_.cells_.begin(); it != table_.cells_.end(); ++it) {
        if (it->second != nullptr) {
            const int c = it->first.col;
            const int r = it->first.row;
            result.rows = std::max(result.rows, r + 1);
            result.cols = std::max(result.cols, c + 1);
        }
    }
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    
    Size size = GetPrintableSize();
    for (int r = 0; r < size.rows; ++r) {

        for (int c = 0; c < size.cols; ++c) {
            if (c > 0) {
                output << "\t";
            }
            const auto& it = table_.cells_.find({ r, c });
            if (it != table_.cells_.end() && it->second && !it->second->GetText().empty()) {
                std::visit([&](const auto value) {output << value; }, it->second->GetValue());
            }
        }
        output << "\n";
    }
}
            
void Sheet::PrintTexts(std::ostream& output) const {
   
    Size size = GetPrintableSize();
    for (int r = 0; r < size.rows; ++r) {

        for (int c = 0; c < size.cols; ++c) {
            if (c > 0) {
                output << "\t";
            }
            const auto& it = table_.cells_.find({ r, c });
            if (it != table_.cells_.end() && it->second && !it->second->GetText().empty()) {
                output << it->second->GetText();
            }
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
