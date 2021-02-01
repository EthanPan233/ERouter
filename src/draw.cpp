#include "draw.h"
#include <stdio.h>
#include "graphics.h"
#include <iostream>

void drawCircuit(Circuit circuit) {

    /* initialize display */
    init_graphics("One Step Router -- Ethan Pan");

    // /* still picture drawing allows user to zoom, etc. */
    // init_world (0.,0.,1000.,1000.);
    // update_message("Interactive graphics example.");
    // event_loop(button_press, drawscreen);   

    /* animation section */
    clearscreen();
    if (mode == std::string("maze")){
        update_message("Maze Routing");
    } else {
        update_message("A* Routing");
    }
    setcolor (RED);
    setlinewidth(0);
    setlinestyle (DASHED);
    init_world (0.,0.,2500.,2500.);

    // draw grid
    int xSize = circuit.getXSize();
    int ySize = circuit.getYSize();
    for (int x=0; x<=xSize; x++) {
        setlinestyle (SOLID);
        setlinewidth (3);
        setcolor(BLACK);
        drawline(0.0, (x+2.)*20., ySize*20., (x+2)*20.);
        // drawline (i,10.*i,i+500.,10.*i+10.);
        // flushinput();
        // delay(); 
    }
    for (int y=0; y<=ySize; y++) {
        drawline(y*20., 40., y*20., xSize*20.+40.);
    }

    // draw pins and obstacles
    for (int x=0; x<xSize; x++) {
        for (int y=0; y<ySize; y++) {
            if (circuit.getCell(x,y).getIsObstructed()) {
                drawTile(x, y, BLACK);
                // std::cout << x << " " << y << "obstructed" << std::endl;
            }
            if (circuit.getCell(x,y).getIsRouted()) {
                // drawTile(x,y,RED);
                drawTile(x, y, color_types(circuit.getCell(x,y).getWireIdx()+4));
            }
            if (circuit.getCell(x,y).getIsPin()) {
                drawTile(x, y, color_types(circuit.getCell(x,y).getWireIdx()+4));
            } 
        }
    }


    event_loop(button_press, drawscreen);
}

void drawscreen() {

}

void drawTile(int x, int y, enum color_types COLOR) {
    setcolor(COLOR);
    fillrect (y*20.,x*20.+40.,y*20.+20.,x*20.+20.+40.);
    // std::cout << "draw tile: " << x << " " << y << std::endl;
}

void drawNet(Circuit circuit, int netIdx) {
    auto ptr = circuit.getWires()[netIdx].begin();
    while (ptr != circuit.getWires()[netIdx].end()) {
        drawTile(ptr->getX(), ptr->getY(), RED);
        ptr++;
    }
}