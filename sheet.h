#pragma once

#include <functional>
#include <vector>
#include <map>
#include "cell.h"
#include "common.h"

using Table = std::vector<std::vector<CellPtr>>;

class Sheet : public SheetInterface {
public:
    ~Sheet(){};

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
          CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    bool IsPositionInsideSheet(const Position& pos) const;

private:
	Table cells_;

    std::map<Position, std::set<Position>> pos_to_refs;
    std::map<CellPtr, std::set<CellPtr>> cell_to_deps;

    void    MakeEmptyCell(Position pos);
    CellPtr MakeEmptyCell();

    void DeleteCell(CellPtr& cell, Position pos = Position::NONE);

    void SetCellRefs(CellPtr cell, Position pos){
        std::set<Position> refs;

        FillCellRefs(refs, cell.get());

        pos_to_refs[pos] = std::move(refs);
    }

    void SetCellDependants(CellPtr cell, Position pos){
        for(const auto ref_pos : pos_to_refs[pos]){
            cell_to_deps[
                cells_[ref_pos.row][ref_pos.col]
                ].insert(cell);
        }
    }

    void FillCellRefs(std::set<Position>& set_to_fill, CellInterface* cell){
        for(const auto ref_pos : cell->GetReferencedCells()){
            
            set_to_fill.insert(ref_pos);

            if(!GetCell(ref_pos))
                MakeEmptyCell(ref_pos);

            if(auto it = pos_to_refs.find(ref_pos); it != pos_to_refs.end()){
                set_to_fill.insert(it->second.begin(), it->second.end());
                continue;
            }
            
            if(!GetCell(ref_pos)->GetReferencedCells().empty()){
                FillCellRefs(set_to_fill, GetCell(ref_pos));
            }
        }
    }
};