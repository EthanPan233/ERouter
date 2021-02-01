# include "Circuit.h"
// # include <string>
# include <fstream>
# include <sstream>
# include <iostream>
// # include <boost/format.hpp>
# include <stdexcept>
# include <algorithm>
# include <cassert>
# include <cstdlib>
# include "draw.h"

void Circuit::routeCell(const int& x, const int& y, const int& wireIdx) {
    cells[x][y].routeCell(wireIdx);
}

void Circuit::readFromFile(const std::string& filename) {
    std::ifstream infile(filename);

    std::string line;

    // initialize grid
    std::getline(infile, line); // The first line is the grid size
    std::istringstream issGridSize(line);
    int xSize, ySize;
    issGridSize >> xSize >> ySize;
    for (int x=0; x<xSize; ++x) {
        cells.push_back(std::vector<Cell>());
        for (int y=0; y<ySize; ++y) {
            cells[x].push_back(Cell(x, y));
        }
    }
    predCells = std::vector<std::vector<std::pair<int, int>>>(xSize, std::vector<std::pair<int, int>>(ySize, std::pair<int, int>(-1,-1)));

    // add obstacles
    std::getline(infile, line);
    std::istringstream issNumObstacle(line);
    int numObstacles;
    issNumObstacle >> numObstacles;
    for (int i=0; i<numObstacles; ++i) {
        std::getline(infile, line);
        std::istringstream iss(line);
        int x, y;
        iss >> x >> y;
        cells[x][y].obstructCell();
    }

    // add wires
    try{
    int numWires = 0;
    std::getline(infile, line);
    std::istringstream issNumWires(line);
    issNumWires >> numWires;
    wires = std::vector<std::vector<Cell>>(numWires, std::vector<Cell>());
    // std::cout << wires.size() << std::endl;
    for (int wireIdx=0; wireIdx < numWires; ++wireIdx){
        std::getline(infile, line);
        std::istringstream iss(line);
        pinOfWires.push_back(std::vector<Cell>()); // make a new wire vector

        int numPins;
        iss >> numPins;
        for (int i=0; i<numPins; ++i){ // for each pin
            int x, y;
            if (!(iss >> x >> y)) { throw std::runtime_error("Too many pins in wire!"); } // error
            routeCell(x, y, wireIdx); // make this cell as pin
            pins.push_back(cells[x][y]);
            cells[x][y].pinCell();
            pinOfWires[wireIdx].push_back(cells[x][y]); // add this pin to wire vector
        }
    }
    }catch(std::exception e){
        std::cout << e.what() << std::endl;
    }

    // drawCircuit(*this);
}

void Circuit::printCircuit() const {
    for (auto rowCells : cells){
        for (auto cell : rowCells){
            std::cout << cell.getState() << " ";
        }
        std::cout << std::endl;
    }
}

void Circuit::printFreeCells() const {
    for (auto rowCells : cells){
        for (auto cell : rowCells){
            std::cout << cell.getIsFree() << " ";
        }
        std::cout << std::endl;
    }
}

bool Circuit::isPin(const Cell& cell) {
    bool isPin = false;
    for (auto pin : pins) {
        if (pin.getX() == cell.getX() && pin.getY() == cell.getY()) return true;
    }
    return false;
}

bool Circuit::isRouted(const int& wireIdx) {
    if (wireIdx > wires.size()) return false;

    // Traveese all cells in a wire to see if they
    // route all pins in a wire
    for (auto pin : pinOfWires[wireIdx]) {
        bool isPinOnWire = false;
        for (auto cellOfWire : wires[wireIdx]) {
            if (cellOfWire.getX() == pin.getX() && cellOfWire.getY() == pin.getY()){
                isPinOnWire = true;
            }
        }
        if (isPinOnWire == false) return false;
    }
    return true;
}

bool Circuit::isRouted(const Cell& cell) {
    try{ // The input can only be a pin, not a commom cell
        if (isPin(cell) == false) throw "Error in isRouted(): the input should be a pin!";
    } catch (std::string exception){
        std::cout << exception << std::endl;
    }

    int wireIdx = cell.getWireIdx();

    return isRouted(wireIdx);
}

void Circuit::ripUpAll() {
    std::cout << "Rip up all nets!" << std::endl;
    for (auto& i : cells){
        for (auto& j : i){
            j.ripUpCell();
        }
    }
    clearLabels();
    for (auto &itr:wires){
        itr.clear();
    }
}

bool compareRouteDiffGreater(std::pair<int,int> diff1, std::pair<int,int> diff2) {
    return diff1.second > diff2.second;
}

bool Circuit::oneStepMaze() {

    std::cout << "Start Maze Routing!" << std::endl;
    ripUpAll();

    int numNets = pinOfWires.size();
    std::cout << "There are totally " << numNets << " nets to route" << std::endl;

    std::vector<std::pair <int,int>> routeDiff(numNets); // netId -> routeDiff
    for (int i = 0; i < numNets; i++) {
        routeDiff[i] = std::pair<int,int>{i, 5};
    }
    
    std::sort(routeDiff.begin(), routeDiff.end(), compareRouteDiffGreater);

    int maxRipUpTimes = 20;
    for (int i = 0; i < maxRipUpTimes; i++) {
        std::cout << "Rip-up reroute iteration " << i << std::endl;
        for (auto& net : routeDiff) {
            drawCircuit(*this);
            if (oneStepMazeSingleNet(net.first)){ // a net is reuted successfully, then mark wires on circuit
                // start marking wires
                std::cout << "Finish routing net " << net.first << std::endl;
                if (&net == &routeDiff.back()) {
                    std::cout << "Finish routing all nets in " << i+1 << " iterations !" << std::endl;
                    std::cout << "Sequence of routing: ";
                    for (auto xx:routeDiff) {
                        std::cout << xx.first << " -> ";
                    }
                    std::cout << "finished!" <<std::endl;
                    return true;
                }
            }
            else { // failed to route
                auto itr = std::find(routeDiff.begin(), routeDiff.end(), net);
                std::cout << "Failed to route net " << net.first << std::endl;
                std::cout << std::distance(routeDiff.begin(), itr) << " nets are successfully routed!" << std::endl;
                std::cout << "Sequence of successfully routed nets: ";
                for (auto xx:routeDiff){
                    if (xx == net){
                        break;
                    }
                    std::cout << xx.first << " -> ";
                }
                std::cout << "failed!" << std::endl;
                std::cout << "Failed to route net " << net.first << std::endl;
                net.second++; // add difficulty
                std::sort(routeDiff.begin(), routeDiff.end(), compareRouteDiffGreater);
                ripUpAll(); // rip up all nets
                break; //reroute
            }
        }
        // return true;
    }
    return false;
}

void labelNeighborCells(std::vector<std::vector<int> > &labelTable, std::vector<std::vector<int>> & preLableTable, int step) {
    preLableTable = labelTable;
    for (int i=0; i<labelTable.size(); i++) {
        for (int j=0; j<labelTable[i].size(); j++) {
            if (labelTable[i][j]==step) {
                if ((i-1)>=0){
                    if (labelTable[i-1][j]==0) {
                        labelTable[i-1][j] = step+1;
                        drawTile(i-1,j,LIGHTGREY);
                    }
                }
                if ((i+1)<labelTable.size()) {
                    if (labelTable[i+1][j]==0) {
                        labelTable[i+1][j] = step+1;
                        drawTile(i+1,j,LIGHTGREY);
                    }
                }
                if ((j-1)>=0){
                    if (labelTable[i][j-1]==0) {
                        labelTable[i][j-1] = step+1;
                        drawTile(i,j-1,LIGHTGREY);
                    }
                }
                if ((j+1)<labelTable[i].size()) {
                    if (labelTable[i][j+1]==0) {
                        labelTable[i][j+1] = step+1;
                        drawTile(i,j+1,LIGHTGREY);
                    }
                }
            }
        }
    }
}


bool labelFinished(std::vector<std::vector<int> > &labelTable, std::vector<std::vector<int> > &preLabelTable) {
    // std::cout << "label finished" << std::endl;
    if (preLabelTable == labelTable) {
        return true;
    } else {
        return false;
    }
}

bool routeFinished(std::vector<std::vector<int> > &labelTable) {
    for (int i=0; i<labelTable.size(); i++) {
        for (int j=0; j<labelTable[i].size(); j++) {
            if (labelTable[i][j]==1||labelTable[i][j]==9999) { // if it's a pin
                if ((j-1>=0&&labelTable[i][j-1]<1)&&((j+1<labelTable[i].size())<labelTable[i][j+1]<1)&&(i-1>=0&&labelTable[i-1][j]<1)&&(i+1<labelTable.size()&&labelTable[i+1][j]<1)) {
                    return false;
                }
            }
        }
    }
    return true;
}

void printLatbelTable(const std::vector<std::vector<int> > &labelTable) {
    // print labelTable
    std::cout << "*************************************" << std::endl;
    for (int i = 0; i < labelTable.size(); i++) {
        for (int j = 0; j < labelTable[i].size(); j++) {
            std::cout << labelTable[i][j] << "   ";
        }
        std::cout << std::endl;
    }
    std::cout << "*************************************" << std::endl;
}

void markWires(const std::vector<std::vector<int>> &labelTable) {

}

bool Circuit::oneStepMazeSingleNet(const int& netIdx) {
    std::cout << "Start Routing net " << netIdx << std::endl;
    std::cout << "There are " << pinOfWires[netIdx].size() << " pins on this net." << std::endl;
    // init a lable table with 0s
    std::vector<std::vector<int>> labelTable(cells.size(), std::vector<int>(cells[0].size(), 0));
    std::vector<std::vector<int>> preLabelTable(cells.size(), std::vector<int>(cells[0].size(), 0));
    std::vector<std::pair<int, int>> targets; // Store all the target pins
    // mark obstacles
    for (int i=0; i<labelTable.size(); i++) {
        for (int j=0; j<labelTable[i].size(); j++){
            if (!cells[i][j].getIsFree()) {
                labelTable[i][j] = -1;
            }
        }
    }

    labelTable[pinOfWires[netIdx][0].getX()][pinOfWires[netIdx][0].getY()] = 1; // source
    std::pair<int, int> source(pinOfWires[netIdx][0].getX(), pinOfWires[netIdx][0].getY());

    for (int i=1; i<pinOfWires[netIdx].size(); i++) {
        labelTable[pinOfWires[netIdx][i].getX()][pinOfWires[netIdx][i].getY()] = 9999; // target
        targets.push_back(std::pair<int, int>(pinOfWires[netIdx][i].getX(), pinOfWires[netIdx][i].getY()));
    }


    int step = 0;
    while (step < 200) {
        step++;
        std::cout << "step " << step << std::endl;
        // printLatbelTable(labelTable);
 
        if (routeFinished(labelTable)) {
            // if the routing is completed!
            // mark wires in the circuit
            int currentX = 0; int currentY = 0;
            int preX = 0; int preY = 0;
            for (auto target : targets) {
                currentX = target.first; currentY = target.second;
                preX = currentX; preY = currentY;
                cells[currentX][currentY].routeCell(netIdx);
                while (!(currentX==source.first && currentY==source.second)) { 
                    // drawTile(currentX, currentY, RED);
                    // flushinput();
                    // delay();
                    // printLatbelTable(labelTable);
                    std::cout << "(" << preX << "," << preY << ") -> (" << currentX << ", " << currentY <<")" << std::endl;
                    // trace from the target back to the source
                    if (labelTable[currentX][currentY]==9999) { // if it's the target
                        int min = 999999; // find the minimum label surround target
                        if (currentY+1<labelTable[currentX].size() && labelTable[currentX][currentY+1]<min && labelTable[currentX][currentY+1]>0) {
                            min = labelTable[currentX][currentY+1];
                        } else if (currentY-1>=0 && labelTable[currentX][currentY-1]<min && labelTable[currentX][currentY-1]>0) {
                            min = labelTable[currentX][currentY-1];
                        } else if (currentX+1<labelTable.size() && currentX+1<labelTable.size() && labelTable[currentX+1][currentY]<min && labelTable[currentX+1][currentY]>0) {
                            min = labelTable[currentX+1][currentY];
                        } else if (currentX-1>=0 && labelTable[currentX-1][currentY]<min && labelTable[currentX-1][currentY]>0) {
                            min = labelTable[currentX-1][currentY];
                        }
                        // mark the surrounding cell which has the smallest label (or the cell on the same wire)
                        if (currentY+1<labelTable[currentX].size() && (labelTable[currentX][currentY+1] == min || cells[currentX][currentY+1].getWireIdx()==netIdx)) { 
                            preX = currentX; preY = currentY; currentY++; cells[currentX][currentY].routeCell(netIdx); wires[netIdx].push_back(cells[currentX][currentY]);
                        } else if (currentY-1>=0 && (labelTable[currentX][currentY-1] == min || cells[currentX][currentY-1].getWireIdx()==netIdx)) {
                            preX = currentX; preY = currentY; currentY--; cells[currentX][currentY].routeCell(netIdx); wires[netIdx].push_back(cells[currentX][currentY]);
                        } else if (currentX+1<labelTable.size() && (labelTable[currentX+1][currentY] == min || cells[currentX+1][currentY].getWireIdx()==netIdx)) {
                            preX = currentX; preY = currentY; currentX++; cells[currentX][currentY].routeCell(netIdx); wires[netIdx].push_back(cells[currentX][currentY]);
                        } else if (currentX-1 >=0 && (labelTable[currentX-1][currentY] == min || cells[currentX-1][currentY].getWireIdx()==netIdx)) {
                            preX = currentX; preY = currentY; currentX--; cells[currentX][currentY].routeCell(netIdx); wires[netIdx].push_back(cells[currentX][currentY]);
                        }
                    } else { // if it's not the target
                        // first see if the neighboring cells are already routed by the same net
                        if (currentY+1<labelTable[currentX].size() && cells[currentX][currentY+1].getWireIdx()==netIdx && labelTable[currentX][currentY+1]!=9999 && currentY+1 != preY)  { preX = currentX; preY = currentY; currentY++; break; }
                        else if (currentY-1>=0 && cells[currentX][currentY-1].getWireIdx()==netIdx && labelTable[currentX][currentY-1]!=9999 && currentY-1 != preY) { preX = currentX; preY = currentY; currentY--; break; }
                        else if (currentX+1<labelTable.size() && cells[currentX+1][currentY].getWireIdx()==netIdx && labelTable[currentX+1][currentY]!=9999 && currentX+1 != preX) { preX = currentX; preY = currentY; currentX++; break; }
                        else if (currentX-1>=0 && cells[currentX-1][currentY].getWireIdx()==netIdx && labelTable[currentX-1][currentY]!=9999 && currentX-1 != preX) { preX = currentX; preY = currentY; currentX--; break; }
                       // then check if the neighboring cells are free
                        else if (currentY+1<labelTable[currentX].size() && (cells[currentX][currentY+1].getIsFree()==true && 
                                labelTable[currentX][currentY+1]==labelTable[currentX][currentY]-1)) {
                            preX = currentX; preY = currentY;
                            currentY++; // move current cell
                            cells[currentX][currentY].routeCell(netIdx); // then route it
                            wires[netIdx].push_back(cells[currentX][currentY]);
                        } else if (currentY-1>=0 && (cells[currentX][currentY-1].getIsFree()==true && 
                                labelTable[currentX][currentY-1]==labelTable[currentX][currentY]-1)) {
                            preX = currentX; preY = currentY;
                            currentY--; // move current cell
                            cells[currentX][currentY].routeCell(netIdx); // then route it
                            wires[netIdx].push_back(cells[currentX][currentY]);
                        } else if (currentX+1<labelTable.size() && (cells[currentX+1][currentY].getIsFree()==true && 
                                labelTable[currentX+1][currentY]==labelTable[currentX][currentY]-1)) {
                            preX = currentX; preY = currentY;
                            currentX++; // move current cell
                            cells[currentX][currentY].routeCell(netIdx); // then route it
                            wires[netIdx].push_back(cells[currentX][currentY]);
                        } else if (currentX-1>=0 && (cells[currentX-1][currentY].getIsFree()==true && 
                                labelTable[currentX-1][currentY]==labelTable[currentX][currentY]-1)) {
                            preX = currentX; preY = currentY;
                            currentX--; // move current cell
                            cells[currentX][currentY].routeCell(netIdx); // then route it
                            wires[netIdx].push_back(cells[currentX][currentY]);
                        }
                    }
                }
            }
            // print labelTable
            // printLatbelTable(labelTable);
            for (auto& cell : wires[netIdx]) {
                // std::cout << cell.getX() << " " << cell.getY() << std::endl;
                drawTile(cell.getX(), cell.getY(), RED);
            }
            return true;
        } else if (labelFinished(labelTable, preLabelTable)) {
            // std::cout << "Failed to route net " << netIdx << std::endl;
            return false;
        } else {
            labelNeighborCells(labelTable, preLabelTable, step);
            flushinput();
            delay();
        }
    }
    std::cout << "Error in function: oneStepMazeSingleNet()" << std::endl;
    // printLatbelTable(labelTable);
    return false;
}

void Circuit::printCellLables() const {
    std::cout << "************************" << std::endl;
    for (auto i : cells) {
        for (auto cell : i) {
            std::cout << cell.getLabel() << "   ";
        }
        std::cout << std::endl;
    }
    std::cout << "************************" << std::endl;
}

void Circuit::clearLabels() {
    std:: cout << "clear labels" << std::endl;
    for (auto& i : cells) {
        for (auto& cell : i) {
            // std::cout << cell.getState() << "   ";
            if (cell.getIsFree()){
                cell.setLabel(-4);
            } else if (cell.getIsRouted()) {
                cell.setLabel(-3);
            } else if (cell.getIsPin()){
                cell.setLabel(-2);
            } else if (cell.getIsObstructed()) {
                cell.setLabel(-1);
            } else {
                std::cout << "!!! clear error !!!" << std::endl;
            }
        }
        // std::cout << std::endl;
    }
}

bool Circuit::oneStepAStar() {
    std::cout << "Start A* Routing ..." << std::endl;
    std::cout << "Grid size: " << cells.size() << " * " << cells[0].size() << std::endl;
    // drawCircuit(*this);
    ripUpAll();
    clearLabels(); //first set labels to 0s
    // printCellLables();

    int numNets = pinOfWires.size();
    std::cout << "There are totally " << numNets << " nets to route" << std::endl;

    std::vector<std::pair <int,int>> routeDiff(numNets); // netId -> routeDiff
    for (int i = 0; i < numNets; i++) {
        routeDiff[i] = std::pair<int,int>{i, 5};
    }
    
    std::sort(routeDiff.begin(), routeDiff.end(), compareRouteDiffGreater);

    int maxRipUpTimes = 20;
    for (int i = 0; i < maxRipUpTimes; i++) {
        std::cout << "Rip-up reroute iteration " << i << std::endl;
        // drawCircuit(*this);
        for (auto& net : routeDiff) {
            // drawCircuit(*this);
            if (oneStepAStarSingleNet(net.first)){ // a net is reuted successfully, then mark wires on circuit
                // start marking wires
                std::cout << "Finish routing net " << net.first << std::endl;
                if (&net == &routeDiff.back()) {
                    std::cout << "Finish routing all nets in " << i+1 << " iterations !" << std::endl;
                    std::cout << "Sequence of routing: ";
                    for (auto xx:routeDiff) {
                        std::cout << xx.first << " -> ";
                    }
                    std::cout << "finished!" <<std::endl;
                    return true;
                }
            }
            else { // failed to route
                auto itr = std::find(routeDiff.begin(), routeDiff.end(), net);
                std::cout << "Failed to route net " << net.first << std::endl;
                std::cout << std::distance(routeDiff.begin(), itr) << " nets are successfully routed!" << std::endl;
                std::cout << "Sequence of successfully routed nets: ";
                for (auto xx:routeDiff){
                    if (xx == net){
                        break;
                    }
                    std::cout << xx.first << " -> ";
                }
                std::cout << "failed!" << std::endl;
                net.second++; // add difficulty
                std::sort(routeDiff.begin(), routeDiff.end(), compareRouteDiffGreater);
                ripUpAll(); // rip up all nets
                break; //reroute
            }
        }
        // return true;
    }
    return false;
}

bool Circuit::targetsRouted(const std::vector<Cell>& targets) const {
    for (auto target : targets) {
        if (cells[target.getX()+1][target.getY()].getLabel()<1 && cells[target.getX()-1][target.getY()].getLabel()<1 
            && cells[target.getX()][target.getY()+1].getLabel()<1 && cells[target.getX()][target.getY()-1].getLabel()<1) {
            return false;
        }
    }
    return true;
}

bool Circuit::targetsRouted(const Cell& target) const {
    if (cells[target.getX()+1][target.getY()].getLabel()<0 && cells[target.getX()-1][target.getY()].getLabel()<0 
        && cells[target.getX()][target.getY()+1].getLabel()<0 && cells[target.getX()][target.getY()-1].getLabel()<0) {
        return false;
    }
    return true;
}

int Circuit::calDistance(const int& sourceX, const int& sourceY, const int& targetX, const int& targetY) const {
    return abs(targetX - sourceX) + abs(targetY - sourceY);
}

bool Circuit::finishLabeling(const std::vector<std::vector<int>> &preLableTable) const {
    for (int i=0; i<cells.size(); i++) {
        for (int j=0; j<cells[0].size(); j++) {
            if (cells[i][j].getLabel() != preLableTable[i][j]) {
                return false;
            }
        }
    }
    return true;
}

void Circuit::calEstDis(const int& x, const int& y) {
    for (int i=0; i<cells.size(); i++) {
        for (int j=0; j<cells[i].size(); j++) {
            if (!(x==i && y==j)) {
                cells[i][j].setestDis(calDistance(i,j,x,y));
            }
        }
    }
}

void Circuit::aStarMarkWires(const int& netIdx, const int& srcX, const int& srcY, const int& tarX, const int& tarY) {
    // if the routing is completed!
    // mark wires in the circuit
    int curX = tarX; int curY = tarY;
    // cells[curX][curY].routeCell(netIdx);
    routeCell(curX, curY, netIdx);

    if (cells[curX][curY+1].getLabel()>0 && (cells[curX][curY+1].getIsFree() || cells[curX][curY+1].getWireIdx() == netIdx)) {
        curY++;
        wires[netIdx].push_back(cells[curX][curY]);
    } else if (cells[curX][curY-1].getLabel()>0 &&( cells[curX][curY-1].getIsFree() || cells[curX][curY-1].getWireIdx() == netIdx)) {
        curY--;
        wires[netIdx].push_back(cells[curX][curY]);
    } else if (cells[curX+1][curY].getLabel()>0 &&( cells[curX+1][curY].getIsFree() || cells[curX+1][curY].getWireIdx() == netIdx)) {
        curX++;
        wires[netIdx].push_back(cells[curX][curY]);
    } else if (cells[curX-1][curY].getLabel()>0 &&( cells[curX-1][curY].getIsFree() || cells[curX-1][curY].getWireIdx() == netIdx)) {
        curX--;
        wires[netIdx].push_back(cells[curX][curY]);
    }

    while (!(curX==srcX && curY==srcY)) { 
        // trace from the target back to the source

        if (cells[curX][curY].getWireIdx() == netIdx){
            break;
        }
        routeCell(curX, curY, netIdx);
        int predX = predCells[curX][curY].first; int predY = predCells[curX][curY].second;
        wires[netIdx].push_back(cells[curX][curY]);
        std::cout << "(" << predX << "," << predY << ") -> (" << curX << ", " << curY <<")" << std::endl;
        curX = predX;
        curY = predY;
    }
}


std::vector<Cell> Circuit::spreadLabel(const int& x, const int& y, const int& netIdx, const int& sourceX, const int& sourceY,
                                        const int& targetX, const int& targetY) {

    std::vector<Cell> result;
    // printFreeCells();
    if (y+1<cells[0].size()){
        if (cells[x][y+1].getWireIdx()==netIdx && cells[x][y+1].getLabel()==-3){
            cells[x][y+1].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x][y+1].getEstDis();
            setLabel(currentDis, x, y+1);
            result.push_back(cells[x][y+1]);
            // predCells[x][y+1] = cells[x][y];
        } else if ((cells[x][y+1].getLabel()==-4) && cells[x][y+1].getIsFree()) {
            cells[x][y+1].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x][y+1].getEstDis();
            setLabel(currentDis, x, y+1);
            result.push_back(cells[x][y+1]);
            predCells[x][y+1].first = x; predCells[x][y+1].second = y;
        }
    }
    if (y-1>=0){
        if (cells[x][y-1].getWireIdx()==netIdx && cells[x][y-1].getLabel()==-3){
            cells[x][y-1].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x][y-1].getEstDis();
            setLabel(currentDis, x, y-1);
            result.push_back(cells[x][y-1]);
            // predCells[x][y-1] = cells[x][y];
        } else if ((cells[x][y-1].getLabel()==-4) && cells[x][y-1].getIsFree()) {
            cells[x][y-1].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x][y-1].getEstDis();
            setLabel(currentDis, x, y-1);
            result.push_back(cells[x][y-1]);
            predCells[x][y-1].first = x; predCells[x][y-1].second = y;
        }
    }
    if (x+1<cells.size()){
        if (cells[x+1][y].getWireIdx()==netIdx && cells[x+1][y].getLabel()==-3){
            cells[x+1][y].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x+1][y].getEstDis();
            setLabel(currentDis, x+1, y);
            result.push_back(cells[x+1][y]);
            // predCells[x+1][y] = cells[x][y];
        } else if ((cells[x+1][y].getLabel()==-4) && cells[x+1][y].getIsFree()) {
            cells[x+1][y].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x+1][y].getEstDis();
            setLabel(currentDis, x+1, y);
            result.push_back(cells[x+1][y]);
            predCells[x+1][y].first = x; predCells[x+1][y].second = y;
        }
    }
    if (x-1>=0){
        if (cells[x-1][y].getWireIdx()==netIdx && cells[x-1][y].getLabel()==-3){
            cells[x-1][y].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x-1][y].getEstDis();
            setLabel(currentDis, x-1, y);
            result.push_back(cells[x-1][y]);
            // predCells[x-1][y] = cells[x][y];
        } else if ((cells[x-1][y].getLabel()==-4) && cells[x-1][y].getIsFree()) {
            cells[x-1][y].setDisToSrc(cells[x][y].getDisToSrc()+1);
            int currentDis = cells[x][y].getDisToSrc() + 1 + cells[x-1][y].getEstDis();
            setLabel(currentDis, x-1, y);
            result.push_back(cells[x-1][y]);
            predCells[x-1][y].first = x; predCells[x-1][y].second = y;
        }
    }
    std::cout << "Spread cells: (" << cells[x][y].getX() << "," << cells[x][y].getY() << ") -> ";
    for (auto cell : result){ 
        std::cout << "(" << cell.getX() << ", " << cell.getY() << ") ";

        drawTile(cell.getX(), cell.getY(), LIGHTGREY);
    }
    std::cout<< std::endl;
    // printCellLables();
    // std::cout << cells[x][y+1].getLabel() << " " <<cells[x][y-1].getLabel() << " " << cells[x+1][y].getLabel() << " " << cells[x-1][y].getLabel() << std::endl;
    // std::cout << cells[x][y+1].getIsRouted() << " " <<cells[x][y-1].getIsRouted() << " " << cells[x+1][y].getIsRouted() << " " << cells[x-1][y].getIsRouted() << std::endl;
    return result;
}

bool Circuit::oneStepAStarSingleNet(const int& netIdx) {
    std::cout << "Start routing net " << netIdx << std::endl;
    // set source and targets
    Cell source = pinOfWires[netIdx][0];
    int sourceX = source.getX();
    int sourceY = source.getY();
    std::cout << "source: " <<sourceX << " " << sourceY << std::endl;
    std::vector<Cell> targets(pinOfWires[netIdx].size()-1);
    for (int i=1; i<pinOfWires[netIdx].size(); i++) {
        targets[i-1] = pinOfWires[netIdx][i];
        std::cout << "target: " << i-1 << ": " << targets[i-1].getX() << " " << targets[i-1].getY() << std::endl;
    }
   
    for (auto const& target : targets){ // route each target in sequence
        drawCircuit(*this);
        int targetX = target.getX();
        int targetY = target.getY();
        // set labels
        clearLabels();

        setLabel(9999, targetX, targetY); // set target label yo 9999

        // calculate estimated distances
        calEstDis(targetX, targetY);
        setLabel(cells[sourceX][sourceY].getEstDis(), sourceX, sourceY);
        cells[sourceX][sourceY].setDisToSrc(0);

        // printCellLables();

        std::vector<Cell> labeledCells;
        std::vector<Cell> edgeCells;
        labeledCells.push_back(cells[sourceX][sourceY]);
        edgeCells.push_back(cells[sourceX][sourceY]);

        std::vector<Cell> waitingCells; // Cells waiting for labeling
        waitingCells.push_back(cells[sourceX][sourceY]);

        // int minDistance = calDistance(sourceX, sourceY, sourceX, ourceY, targetX, targetY);

        int step = 0;
        int failTime = 0;
        while (step < 300) {
            // printCellLables();
            flushinput();
            delay();
            step++;
            std::cout << "step: " << step << std::endl;
            std::vector<std::vector<int>> preLableTable(cells.size(), std::vector<int>(cells[0].size(),0));
            if (targetsRouted(target)){
                // printCellLables();
                aStarMarkWires(netIdx, sourceX, sourceY, targetX, targetY);
                std::cout << "Finish routing net: " << netIdx << ", (" << sourceX << ", " << sourceY << ") -> (" << targetX << ", " << targetY << ")" << std::endl;
                // drawNet(*this, netIdx);
                break;
            } else if(finishLabeling(preLableTable)){
                failTime++;
                if (failTime > 5) {
                    std::cout << "Failed to route net: " << netIdx << ", (" << sourceX << ", " << sourceY << ") -> (" << targetX << ", " << targetY << ")" << std::endl;
                    return false;
                }
            } else {
                int minDis = 999999;
                for ( int i=0; i<edgeCells.size(); i++) { // find the smallest distance among edge cells
                    if (edgeCells[i].getLabel()<minDis) minDis = edgeCells[i].getLabel(); 
                }

                std::vector<Cell> newSpreadCells;
                auto cellToSpread = edgeCells.begin();
                std::cout << "There are " << edgeCells.size() << " cells in edge list: ";
                if (edgeCells.size() == 0) {
                    std::cout << std::endl << "Failed to route net " << netIdx << ": (" << sourceX << ", " << sourceY << ") -> (" << targetX << ", " << targetY << ")" << std::endl;
                    return false;
                }
                for (auto cell : edgeCells) { std:: cout << "(" << cell.getX() << "," << cell.getY() << ")"; }
                std::cout << std::endl;
                std::vector<Cell> tempCellList;
                for (int i=0; i<cells.size(); i++) {
                    for (int j=0; j<cells[0].size(); j++) {
                        preLableTable[i][j] = cells[i][j].getLabel();
                    }
                }
                while (cellToSpread!=edgeCells.end()) {
                    if (cellToSpread->getLabel() > minDis) {
                        // std::cout << "cell: (" << cellToSpread->getX() <<", " << cellToSpread->getY() << ") label: " << cellToSpread->getLabel() << " was not spread." << std::endl;
                        cellToSpread++;
                        continue;
                    }
                    else {
                        // std::cout << "cell: (" << cellToSpread->getX() <<", " << cellToSpread->getY() << ") label: " << cellToSpread->getLabel() << " finish spread." << std::endl;
                        tempCellList = spreadLabel(cellToSpread->getX(), cellToSpread->getY(), netIdx, sourceX, sourceY, targetX, targetY);
                        newSpreadCells.insert(newSpreadCells.end(), tempCellList.begin(), tempCellList.end());
                        // printCellLables();
                        edgeCells.erase(cellToSpread);
                        // std::cout << "Spread around one cell, then delete that cell" << std::endl;
                        // cellToSpread++;
                    }
                }
                // std::cout << "Finish spreading" << std::endl;
                edgeCells.insert(edgeCells.end(), newSpreadCells.begin(), newSpreadCells.end());
                // std::cout << "Insert new cells to edge cell list." << std::endl;
           }
        }
    }
    return true;
}