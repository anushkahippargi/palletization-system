#include "PalletStatistics.h"

PalletStatistics::PalletStatistics()
    : palletId(0),
      usedVolume(0.0),
      totalVolume(0.0),
      utilization(0.0)
{
}

PalletStatistics::PalletStatistics(
    int palletId,
    double usedVolume,
    double totalVolume)
    : palletId(palletId),
      usedVolume(usedVolume),
      totalVolume(totalVolume)
{
    updateUtilization();
}

int PalletStatistics::getPalletId() const
{
    return palletId;
}

double PalletStatistics::getUsedVolume() const
{
    return usedVolume;
}

double PalletStatistics::getTotalVolume() const
{
    return totalVolume;
}

double PalletStatistics::getUtilization() const
{
    return utilization;
}

void PalletStatistics::setUsedVolume(
    double usedVolume)
{
    this->usedVolume = usedVolume;

    updateUtilization();
}

void PalletStatistics::setTotalVolume(
    double totalVolume)
{
    this->totalVolume = totalVolume;

    updateUtilization();
}

void PalletStatistics::updateUtilization()
{
    if (totalVolume == 0)
    {
        utilization = 0;
    }
    else
    {
        utilization =
            (usedVolume / totalVolume) * 100.0;
    }
}