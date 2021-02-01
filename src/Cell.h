#pragma once

// TODO: 1. enum states; 2. implement getState()

class Cell {
    public:
        Cell() { }
        Cell(const int& x, const int& y) { xCoor = x; yCoor = y; }
        // Cell(const Cell& C) { xCoor = C.getX(); yCoor = C.getY(); }
        bool ifUsed() const { return isRouted; }
        void routeCell(const int& wireIdx) { this->isRouted = true; this->isFree = false; this->wireIdx = wireIdx; }
        void obstructCell() { this->isObstructed = true; this->isFree = false; this->label = -1; }
        void pinCell() { this->isPin = true; isFree = false; this->label = -2; }
        void freeCell() { isFree = true; isRouted = isObstructed = isRouted = false; }

        int getX() const { return xCoor; }
        int getY() const { return yCoor; }
        int getState() const;
        // void setWireIdx( const int& wireIdx ) { this->wireIdx = wireIdx; }
        int getWireIdx() const { return wireIdx; }
        bool getIsObstructed() const { return isObstructed; }
        bool getIsRouted() const { return isRouted; }
        bool getIsPin() const { return isPin; }
        bool getIsFree() const { return isFree; }
        int getLabel() const { return label; }
        int getEstDis() const { return estDis; }
        int getDisToSrc() const { return disToSrc; }

        void ripUpCell() { 
            isRouted = false; 
            if ((!isPin) && (!isObstructed)) { 
                wireIdx = -1; isFree = true;
            } 
        }

        void setLabel(const int& label) { this->label = label; }
        void setestDis(const int& estDis) { this->estDis = estDis; }
        void setDisToSrc(const int& disToSrc) { this->disToSrc = disToSrc; }
    

    private:
        bool isRouted = false;
        bool isObstructed = false;
        bool isPin = false;
        bool isFree  = true;
        int xCoor;
        int yCoor;
        int wireIdx = -1; // default is -1, which means not on a wire
        int label = -99; // used in AStar routing, distance between source and target. pin: -2, obstacles: -1
        int estDis = -99;
        int disToSrc = -99;
};