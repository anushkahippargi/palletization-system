#include <iostream>

#include "Box.h"
#include "Pallet.h"
#include "Placement.h"
#include "PalletizationResult.h"
#include "RowAlgorithm.h"

using namespace std;

int main()
{
    Box box(
        300,
        200,
        150,
        5.5);

    Pallet pallet(
    1200,   // length
    1000,   // width
    150,    // pallet height
    1000);  // maximum load weight (kg or your chosen unit)

    RowAlgorithm algorithm;

    PalletizationResult result =
        algorithm.generatePattern(
            pallet,
            box,
            10);

    for (const Placement& placement : result.getPlacements())
    {
        placement.print();
        cout << endl;
    }
    cout << "\nStatistics\n";

const Statistics& statistics =
    result.getStatistics();

cout << "Total Boxes : "
     << statistics.getTotalBoxes()
     << endl;

cout << "Used Pallets : "
     << statistics.getUsedPallets()
     << endl;

for (const PalletStatistics& pallet :
     statistics.getPalletStatistics())
{
    cout << "\nPallet "
         << pallet.getPalletId()
         << endl;

    cout << "Used Volume : "
         << pallet.getUsedVolume()
         << endl;

    cout << "Total Volume : "
         << pallet.getTotalVolume()
         << endl;

    cout << "Utilization : "
         << pallet.getUtilization()
         << "%"
         << endl;
}

    return 0;
}