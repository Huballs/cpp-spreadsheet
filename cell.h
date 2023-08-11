#pragma once


#include <set>
#include "common.h"
#include "formula.h"

class Cell;

using CellPtr = std::shared_ptr<Cell>;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}
    ~Cell() = default;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    
    class Impl {
    public:
        
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const { return {};};
        
        virtual ~Impl() = default;
    };
    
    class EmptyImpl : public Impl {
    public:    
        
        Value GetValue() const override;
        std::string GetText() const override;      
    };
    
    class TextImpl : public Impl {
    public:
        
        explicit TextImpl(std::string text); 
        Value GetValue() const override;       
        std::string GetText() const override;
        
    private:
        std::string text_;        
    };
    
    class FormulaImpl : public Impl {
    public:
        
        explicit FormulaImpl(std::string text, SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;
        
    private:
        std::unique_ptr<FormulaInterface> formula_ptr_;  

        SheetInterface& sheet_;      
    };
    
    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;

    FormulaInterface::Value cache_;
    bool isCacheValid = false;

    void InvalidateCache();
    bool CheckCircularDependecy(Impl& impl);

};
