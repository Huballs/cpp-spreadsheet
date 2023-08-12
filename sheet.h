#pragma once

#include <functional>
#include <vector>
#include <map>
#include "cell.h"
#include "common.h"

struct Table_t{

        CellPtr operator()(Position pos){
            ResizeToFit(pos);
            return cells_[pos.row][pos.col];
        }

        inline void SetCell(CellPtr cell){

            Position pos = cell->GetPosition();

            ResizeToFit(pos);
            cells_[pos.row][pos.col] = cell;
        }

        inline void DeleteCell(Position pos){
            if(!IsPosInside(pos)){
                throw InvalidPositionException("On Table::DeleteCell");
            }
            RemoveCellConnections(pos);

            cells_[pos.row][pos.col].reset();
        }

        inline void RemoveCellConnections(Position pos){

            for(const auto& ref_pos : pos_to_refs[pos]){
                cell_to_deps[ref_pos].erase(pos);
            }
            
            pos_to_refs.erase(pos);
        }

        inline void ResizeToFit(Position pos){
            cells_.resize(std::max(pos.row + 1, static_cast<int>(cells_.size())));
            cells_[pos.row].resize(std::max(pos.col + 1, static_cast<int>(cells_[pos.row].size())));
        }

        inline bool IsPosInside(Position pos) const {
            if  (  pos.row < static_cast<int>(cells_.size()) 
                && pos.col < static_cast<int>(cells_[pos.row].size())){

                return true;
            }
            return false;
        }

        inline CellPtr MakeEmptyCell(Position pos){
            return std::make_shared<Cell>(*this, pos);
        }

        void SetCellConnections(CellPtr cell){
            SetCellRefs(cell);
            SetCellDependants(cell);
        }

        void SetCellRefs(CellPtr cell){

            for(const auto ref_pos : cell->GetReferencedCells()){

                auto ref_cell = (*this)(ref_pos);

                if(!ref_cell){
                    ref_cell = MakeEmptyCell(ref_pos);
                    SetCell(ref_cell);
                }

                pos_to_refs[cell->GetPosition()].insert(ref_pos);
            }

        }

        void SetCellDependants(CellPtr cell){
            for(const auto ref_pos : cell->GetReferencedCells()){
                cell_to_deps[ref_pos].insert(cell->GetPosition());
            }
        }

        bool IsCircularDependency(CellPtr cell){

            std::function<bool (Position, const std::vector<Position>&)> 
                go_up = [&](Position pos, const std::vector<Position>& pos_to_find){

                auto it = cell_to_deps.find(pos);

                if(it != cell_to_deps.end()){

                    if(it->second.empty()){
                        cell_to_deps.erase(it);
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

        std::vector<std::vector<CellPtr>> cells_;

        std::map<Position, std::set<Position>> pos_to_refs;
        std::map<Position, std::set<Position>> cell_to_deps;

    } ;

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
	Table_t table;

};