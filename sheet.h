#pragma once

#include <functional>
#include <vector>
#include <map>
#include "cell.h"
#include "common.h"

struct Table{

    CellPtr operator()(Position pos);

    inline void SetCell(CellPtr cell);

    inline void DeleteCell(Position pos);

    inline void RemoveCellConnections(Position pos);

    inline void ResizeToFit(Position pos);
    inline bool IsPosInside(Position pos) const;

    std::vector<std::vector<CellPtr>> cells_;

    std::map<Position, std::set<Position>> pos_to_refs;
    std::map<Position, std::set<Position>> cell_to_deps;

};

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

private:
	Table table_;

    CellPtr MakeEmptyCell(Position pos);

    void SetCellConnections(CellPtr cell){
        SetCellRefs(cell);
        SetCellDependants(cell);
    }

    void SetCellRefs(CellPtr cell){

        for(const auto ref_pos : cell->GetReferencedCells()){

            auto ref_cell = table_(ref_pos);

            if(!ref_cell){
                ref_cell = MakeEmptyCell(ref_pos);
                table_.SetCell(ref_cell);
            }

            table_.pos_to_refs[cell->GetPosition()].insert(ref_pos);
        }

    }

    void SetCellDependants(CellPtr cell){
        for(const auto ref_pos : cell->GetReferencedCells()){
            table_.cell_to_deps[ref_pos].insert(cell->GetPosition());
        }
    }

    bool IsCircularDependency(CellPtr cell){

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
};