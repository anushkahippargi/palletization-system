#pragma once

class Statistics
{
private:
    int boxesPlaced;
    int boxesRemaining;

    double occupiedArea;
    double utilization;

public:
    Statistics(int boxesPlaced,
               int boxesRemaining,
               double occupiedArea,
               double utilization);

    int getBoxesPlaced() const;
    int getBoxesRemaining() const;

    double getOccupiedArea() const;
    double getUtilization() const;
};