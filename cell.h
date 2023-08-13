#pragma once

#include <set>
#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet, Position position = Position::NONE) 
        : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet), position_(position) {}
   
    ~Cell() = default;

    friend SheetInterface;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    void InvalidateCache();

    Position GetPosition() const;

private:
    
    class Impl {
    public:
        
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const { return {};};

        virtual void InvalidateCache(){};
        
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

        void InvalidateCache() override;

        mutable Cell::Value cache_;
        mutable bool isCacheValid = false;
        
    private:
        std::unique_ptr<FormulaInterface> formula_ptr_;  

        SheetInterface& sheet_;      
    };
    
    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;

    Position position_;

    bool CheckCircularDependecy(Impl& impl);

};
