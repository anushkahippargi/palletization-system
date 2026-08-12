#include "Statistics.h"

Statistics::Statistics()
    : totalBoxes(0),
      fullPallets(0),
      lastPalletStatistics(),
      hasLastPalletStatistics(false)
{
}

int Statistics::getTotalBoxes() const
{
    return totalBoxes;
}

int Statistics::getFullPallets() const
{
    return fullPallets;
}

bool Statistics::hasLastPallet() const
{
    return hasLastPalletStatistics;
}

const PalletStatistics&
Statistics::getLastPalletStatistics() const
{
    return lastPalletStatistics;
}

void Statistics::setTotalBoxes(
    int totalBoxes)
{
    this->totalBoxes = totalBoxes;
}

void Statistics::setFullPallets(
    int fullPallets)
{
    this->fullPallets = fullPallets;
}

void Statistics::setLastPalletStatistics(
    const PalletStatistics& statistics)
{
    this->lastPalletStatistics = statistics;
    this->hasLastPalletStatistics = true;
}