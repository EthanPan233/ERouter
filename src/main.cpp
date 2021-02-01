#include "Cell.h"
#include "Circuit.h"
#include <iostream>
#include "draw.h"
#include <cassert>
// #include "graphics.h"
// #include "OneStepRouteGride.h"

std::string filename;
std::string mode;

int main(int argc, char *argv[]) {

    assert(argc == 3);

    Circuit circuit;

    filename = std::string("../benchmarks/") + argv[1] + std::string(".infile");
    mode = argv[2];
    circuit.readFromFile(filename);
    // drawCircuit(circuit);

    if (argv[2] == std::string("maze")){
        std::cout << "Start Maze routing!" << std::endl;
        circuit.oneStepMaze();
    } else if (argv[2] == std::string("astar")) {
        std::cout << " Start AStart Routing!" << std::endl;
        circuit.oneStepAStar();
    } else {
        std::cout << "argv[2] is " << argv[2] << " but should be \"maze\" or \"astar\"! " << std::endl;
    }
    // circuit.printCircuit();


    int wireLen = 0;
    // std::cout << wires.size() << std::endl;
    for (int ii = 0; ii < circuit.getXSize(); ii++) {
        // std::cout << wires[ii].size() << std::endl;
        for (int jj=0; jj<circuit.getYSize(); jj++) {
            if (circuit.getCell(ii,jj).getIsRouted() || circuit.getCell(ii,jj).getIsPin()){
                wireLen++;
                // std::cout << ii << " " << jj << std::endl;
            }
        }
    }
    std::cout << "Total wire length = " << wireLen << std::endl;

    clearscreen();
    drawCircuit(circuit);

    return 0;
}