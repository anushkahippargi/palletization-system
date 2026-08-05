#pragma once

#include <vector>

#include "Placement.h"
#include "Statistics.h"

using namespace std;

class PalletizationResult
{
private:

    vector<Placement> placements;

    Statistics statistics;

public:

    PalletizationResult() = default;

    vector<Placement>& getPlacements();

    const vector<Placement>& getPlacements() const;

    Statistics& getStatistics();

    const Statistics& getStatistics() const;
};