#pragma once
#include <vector>
#include <string>
#include "Cell.h"

class Circuit : public Cell{
    public:
        // Circuit(const int& x, const int& y);
        Circuit() { }
        Cell getCell(const int& i, const int& j) { return cells[i][j]; }
        int getXSize() const { return cells.size(); }
        int getYSize() const { return cells[0].size(); }
        std::vector<std::vector<Cell>> getWires() {return wires;}

        void routeCell(const int& x, const int& y, const int& wireIdx);
        void routeCell(const Cell& targetCell, const int& wireIdx) {return routeCell(targetCell.getX(), targetCell.getY(), wireIdx);}
        
        void setNumWires(const int& numWires) { this->numWires = numWires; }
        void setLabel(const int& label, const int& x, const int& y) { this->cells[x][y].setLabel(label);}

        bool isRouted(const int& wireIdx); // if a wire is routed
        bool isRouted(const Cell& cell); // if a pin is routed
        bool isPin(const Cell& cell); // if a cell is a pin

        bool oneStepMaze();
        bool oneStepMazeSingleNet(const int& netIdx);

        bool oneStepAStar();
        bool oneStepAStarSingleNet(const int& netIdx);
        bool targetsRouted(const std::vector<Cell>& targets) const;
        bool targetsRouted(const Cell& target) const;
        bool finishLabeling(const std::vector<std::vector<int>>& preLableTable) const;
        void aStarMarkWires(const int& netIdx, const int& srcX, const int& srcY, const int& tarX, const int& tarY);
        std::vector<Cell> spreadLabel(const int& x, const int& y, const int& netIdx, const int& srcX, const int& srcY, const int& targetX, const int& targetY);
        void calEstDis(const int& x, const int& y);

        void ripUpAll();
        void clearLabels();

        void readFromFile(const std::string& filename);

        void printCircuit() const;
        void printCellLables() const;
        void printFreeCells() const;

        int calDistance(const int& x, const int& y, const int& targetX, const int& targetY) const;
        std::vector<std::vector<std::pair<int, int>>> predCells;

        // std::vector<std::pair<int, int>> obstacles;
        std::vector<std::vector<std::vector<std::vector<std::vector<std::pair<int, int>>>>>> aStarLabels;



    private:
        // Status:
        // 0: free
        // 1: occupied
        // 2: obstructed
        // 3: pin
        std::vector<std::vector<Cell>> cells;
        std::vector<std::vector<Cell>> pinOfWires;
        std::vector<Cell> pins;
        std::vector<std::vector<Cell>> wires; // which cells are on wires
        
        int numWires = 0;
};