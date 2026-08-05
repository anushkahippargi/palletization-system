#pragma once

#include <vector>

#include "PalletStatistics.h"

using namespace std;

class Statistics
{
private:

    int totalBoxes;

    int usedPallets;

    vector<PalletStatistics> palletStatistics;

public:

    Statistics();

    int getTotalBoxes() const;

    int getUsedPallets() const;

    vector<PalletStatistics>& getPalletStatistics();

    const vector<PalletStatistics>& getPalletStatistics() const;

    void setTotalBoxes(
        int totalBoxes);

    void setUsedPallets(
        int usedPallets);

    void addPalletStatistics(
        const PalletStatistics& statistics);
};