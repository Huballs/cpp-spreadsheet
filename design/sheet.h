#pragma once

#include <functional>
#include <vector>
#include <map>
#include "cell.h"
#include "common.h"

using CellPtr = std::shared_ptr<Cell>;

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

    void SetCellConnections(CellPtr cell);

    void InvalidateCacheOfDependants(Position pos);

    void SetCellRefs(CellPtr cell);

    void SetCellDependants(CellPtr cell);
    bool IsCircularDependency(CellPtr cell);
};