#include <iostream>

#include "Box.h"
#include "Pallet.h"
#include "Placement.h"
#include "PalletizationResult.h"
#include "PalletStatistics.h"
#include "RowAlgorithm.h"
#include "Statistics.h"

using namespace std;

int main()
{
    Box box(
        300,
        200,
        150,
        5.5);

    Pallet pallet(
        1200,
        1000,
        300,
        1000);

    RowAlgorithm algorithm;

    // Test case:
    // Pallet capacity = 40
    // Requested boxes = 45
    //
    // Expected:
    // Pallet 1 -> 40 boxes -> Full
    // Pallet 2 -> 5 boxes  -> Partial
    PalletizationResult result =
        algorithm.generatePattern(
            pallet,
            box,
            45);

    // Display all placements.
    for (const Placement& placement :
         result.getPlacements())
    {
        placement.print();

        cout << endl;
    }

    // Display statistics.
    cout << "\nStatistics"
         << endl;

    const Statistics& statistics =
        result.getStatistics();

    cout << "Total Boxes : "
         << statistics.getTotalBoxes()
         << endl;

    cout << "Full Pallets : "
         << statistics.getFullPallets()
         << endl;

    // Only display statistics for the final
    // pallet when it is partially filled.
    if (statistics.hasLastPallet())
    {
        const PalletStatistics&
            lastPallet =
                statistics.getLastPalletStatistics();

        cout << "\nLast Pallet"
             << endl;

        cout << "Pallet ID : "
             << lastPallet.getPalletId()
             << endl;

        cout << "Used Volume : "
             << lastPallet.getUsedVolume()
             << endl;

        cout << "Total Volume : "
             << lastPallet.getTotalVolume()
             << endl;

        cout << "Utilization : "
             << lastPallet.getUtilization()
             << "%"
             << endl;
    }

    return 0;
}