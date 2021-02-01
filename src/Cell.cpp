# include "Cell.h"

int Cell::getState() const {
    if (isFree) { return 0; }
    else if (isPin) { return 1; }
    else if (isObstructed) { return 2; }
    else if (isRouted) { return 3; }
    else { return -1; }
}