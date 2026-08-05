#include <iostream>
#include <vector>

#include "Box.h"
#include "Pallet.h"
#include "Placement.h"
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
        1200,
        1000,
        1200,
        1000);

    RowAlgorithm algorithm;

    vector<Placement> placements =
        algorithm.generatePattern(
            pallet,
            box,
            10);

    for (const Placement& placement : placements)
    {
        placement.print();
        cout << endl;
    }

    return 0;
}