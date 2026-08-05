#include "PalletizationResult.h"

vector<Placement>& PalletizationResult::getPlacements()
{
    return placements;
}

const vector<Placement>& PalletizationResult::getPlacements() const
{
    return placements;
}

Statistics& PalletizationResult::getStatistics()
{
    return statistics;
}

const Statistics& PalletizationResult::getStatistics() const
{
    return statistics;
}