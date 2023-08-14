#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include "cell.h"
#include "common.h"

using CellPtr = std::shared_ptr<Cell>;

struct Table{

    CellPtr operator()(Position pos);
    const CellPtr operator()(Position pos) const;

    inline void SetCell(CellPtr cell);

    inline void DeleteCell(Position pos);

    inline void RemoveCellConnections(Position pos);

    struct PHasher {
        size_t operator()(const Position& p) const {
            return std::hash<size_t>()(
                (static_cast<size_t>(p.row) << 32) | static_cast<size_t>(p.col)
                );
        }
    };

    std::unordered_map<Position, CellPtr, PHasher> cells_;

    std::unordered_map<Position, std::set<Position>, PHasher> pos_to_refs;
    std::unordered_map<Position, std::set<Position>, PHasher> cell_to_deps;

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

    void SetCellConnections(CellPtr cell);

    void InvalidateCacheOfDependants(Position pos);

    void SetCellRefs(CellPtr cell);

    void SetCellDependants(CellPtr cell);
    bool IsCircularDependency(CellPtr cell);
};