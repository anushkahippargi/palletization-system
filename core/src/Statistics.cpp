#include "Statistics.h"

Statistics::Statistics()
    : totalBoxes(0),
      usedPallets(0)
{
}

int Statistics::getTotalBoxes() const
{
    return totalBoxes;
}

int Statistics::getUsedPallets() const
{
    return usedPallets;
}

vector<PalletStatistics>& Statistics::getPalletStatistics()
{
    return palletStatistics;
}

const vector<PalletStatistics>& Statistics::getPalletStatistics() const
{
    return palletStatistics;
}

void Statistics::setTotalBoxes(
    int totalBoxes)
{
    this->totalBoxes = totalBoxes;
}

void Statistics::setUsedPallets(
    int usedPallets)
{
    this->usedPallets = usedPallets;
}

void Statistics::addPalletStatistics(
    const PalletStatistics& statistics)
{
    palletStatistics.push_back(statistics);
}