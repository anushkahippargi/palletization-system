#include "Statistics.h"

Statistics::Statistics(int boxesPlaced,
                       int boxesRemaining,
                       double occupiedArea,
                       double utilization)
    : boxesPlaced(boxesPlaced),
      boxesRemaining(boxesRemaining),
      occupiedArea(occupiedArea),
      utilization(utilization) {}

int Statistics::getBoxesPlaced() const
{
    return boxesPlaced;
}

int Statistics::getBoxesRemaining() const
{
    return boxesRemaining;
}

double Statistics::getOccupiedArea() const
{
    return occupiedArea;
}

double Statistics::getUtilization() const
{
    return utilization;
}
