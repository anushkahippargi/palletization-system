#pragma once

#include "PalletStatistics.h"

using namespace std;

class Statistics
{
private:

    int totalBoxes;

    int fullPallets;

    PalletStatistics lastPalletStatistics;

    bool hasLastPalletStatistics;

public:

    Statistics();

    int getTotalBoxes() const;

    int getFullPallets() const;

    bool hasLastPallet() const;

    const PalletStatistics& getLastPalletStatistics() const;

    void setTotalBoxes(
        int totalBoxes);

    void setFullPallets(
        int fullPallets);

    void setLastPalletStatistics(
        const PalletStatistics& statistics);
};