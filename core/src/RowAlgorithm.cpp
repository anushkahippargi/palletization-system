#include <iostream>
#include <vector>

#include "RowAlgorithm.h"

using namespace std;

int RowAlgorithm::calculateBoxesAlongX(
    const Pallet& pallet,
    double boxLength) const
{
    if (boxLength <= 0)
        return 0;

    return static_cast<int>(
        pallet.getLength() / boxLength);
}

int RowAlgorithm::calculateBoxesAlongY(
    const Pallet& pallet,
    double boxWidth) const
{
    if (boxWidth <= 0)
        return 0;

    return static_cast<int>(
        pallet.getWidth() / boxWidth);
}

int RowAlgorithm::calculateBoxesAlongZ(
    const Pallet& pallet,
    double boxHeight) const
{
    if (boxHeight <= 0)
        return 0;

    return static_cast<int>(
        pallet.getHeight() / boxHeight);
}

RowAlgorithm::Orientation
RowAlgorithm::findBestOrientation(
    const Pallet& pallet,
    const Box& box) const
{
    vector<Orientation> orientations =
    {
        {
            box.getLength(),
            box.getWidth(),
            box.getHeight(),
            0,
            0,
            0
        },

        {
            box.getWidth(),
            box.getLength(),
            box.getHeight(),
            0,
            0,
            90
        },

        {
            box.getLength(),
            box.getHeight(),
            box.getWidth(),
            90,
            0,
            0
        },

        {
            box.getHeight(),
            box.getLength(),
            box.getWidth(),
            90,
            0,
            90
        },

        {
            box.getWidth(),
            box.getHeight(),
            box.getLength(),
            0,
            90,
            0
        },

        {
            box.getHeight(),
            box.getWidth(),
            box.getLength(),
            0,
            90,
            90
        }
    };

    Orientation bestOrientation =
        orientations[0];

    int bestCapacity = -1;
    int bestBoxesX = 0;
    int bestBoxesY = 0;
    int bestBoxesZ = 0;

    cout << "\nRO Orientation Optimization"
         << endl;

    for (int i = 0;
         i < static_cast<int>(orientations.size());
         i++)
    {
        const Orientation& orientation =
            orientations[i];

        int boxesX =
            calculateBoxesAlongX(
                pallet,
                orientation.length);

        int boxesY =
            calculateBoxesAlongY(
                pallet,
                orientation.width);

        int boxesZ =
            calculateBoxesAlongZ(
                pallet,
                orientation.height);

        bool valid =
            boxesX > 0 &&
            boxesY > 0 &&
            boxesZ > 0;

        cout << "Orientation "
             << i + 1
             << ": "
             << orientation.length
             << " x "
             << orientation.width
             << " x "
             << orientation.height;

        if (!valid)
        {
            cout << " -> INVALID"
                 << endl;

            continue;
        }

        int capacity =
            boxesX *
            boxesY *
            boxesZ;

        cout << " -> "
             << boxesX
             << " x "
             << boxesY
             << " x "
             << boxesZ
             << " = "
             << capacity
             << endl;

        bool isBetter = false;

        // Rule 1:
        // Higher capacity always wins.
        if (capacity > bestCapacity)
        {
            isBetter = true;
        }
        // Rule 2:
        // If capacity is tied, prefer more Z layers.
        else if (capacity == bestCapacity &&
                 boxesZ > bestBoxesZ)
        {
            isBetter = true;
        }
        // Rule 3:
        // If still tied, prefer more boxes along Y.
        else if (capacity == bestCapacity &&
                 boxesZ == bestBoxesZ &&
                 boxesY > bestBoxesY)
        {
            isBetter = true;
        }

        if (isBetter)
        {
            bestCapacity = capacity;

            bestOrientation =
                orientation;

            bestBoxesX =
                boxesX;

            bestBoxesY =
                boxesY;

            bestBoxesZ =
                boxesZ;
        }
    }

    cout << "\nSelected Orientation: "
         << bestOrientation.length
         << " x "
         << bestOrientation.width
         << " x "
         << bestOrientation.height
         << endl;

    cout << "Rotation X: "
         << bestOrientation.rotationX
         << " degrees"
         << endl;

    cout << "Rotation Y: "
         << bestOrientation.rotationY
         << " degrees"
         << endl;

    cout << "Rotation Z: "
         << bestOrientation.rotationZ
         << " degrees"
         << endl;

    cout << "Boxes along X: "
         << bestBoxesX
         << endl;

    cout << "Boxes along Y: "
         << bestBoxesY
         << endl;

    cout << "Boxes along Z: "
         << bestBoxesZ
         << endl;

    cout << "Maximum Capacity: "
         << bestCapacity
         << endl;

    return bestOrientation;
}

PalletizationResult RowAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;

    double boxVolume =
        box.getLength() *
        box.getWidth() *
        box.getHeight();

    double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();

    Orientation orientation =
        findBestOrientation(
            pallet,
            box);

    int boxesAlongX =
        calculateBoxesAlongX(
            pallet,
            orientation.length);

    int boxesAlongY =
        calculateBoxesAlongY(
            pallet,
            orientation.width);

    int boxesAlongZ =
        calculateBoxesAlongZ(
            pallet,
            orientation.height);

    int palletCapacity =
        boxesAlongX *
        boxesAlongY *
        boxesAlongZ;

    cout << "\nSelected RO Capacity"
         << endl;

    cout << "Boxes along X : "
         << boxesAlongX
         << endl;

    cout << "Boxes along Y : "
         << boxesAlongY
         << endl;

    cout << "Boxes along Z : "
         << boxesAlongZ
         << endl;

    cout << "Pallet Capacity : "
         << palletCapacity
         << endl;

    if (palletCapacity <= 0 ||
        quantity <= 0)
    {
        return result;
    }

    int boxesPlaced = 0;
    int fullPallets = 0;
    int palletId = 1;

    while (boxesPlaced < quantity)
    {
        int boxesOnCurrentPallet = 0;

        for (int zIndex = 0;
             zIndex < boxesAlongZ &&
             boxesPlaced < quantity;
             zIndex++)
        {
            for (int yIndex = 0;
                 yIndex < boxesAlongY &&
                 boxesPlaced < quantity;
                 yIndex++)
            {
                for (int xIndex = 0;
                     xIndex < boxesAlongX &&
                     boxesPlaced < quantity;
                     xIndex++)
                {
                    double x =
                        xIndex *
                        orientation.length;

                    double y =
                        yIndex *
                        orientation.width;

                    double z =
                        zIndex *
                        orientation.height;

                    Matrix4x4 pose;

                    pose.setTranslation(
                        x,
                        y,
                        z);

                    if (orientation.rotationX != 0)
                    {
                        pose.setRotationX(
                            orientation.rotationX);
                    }
                    else if (orientation.rotationY != 0)
                    {
                        pose.setRotationY(
                            orientation.rotationY);
                    }
                    else
                    {
                        pose.setRotationZ(
                            orientation.rotationZ);
                    }

                    Placement placement(
                        boxesPlaced + 1,
                        palletId,
                        pose);

                    result.getPlacements()
                        .push_back(placement);

                    boxesPlaced++;
                    boxesOnCurrentPallet++;
                }
            }
        }

        if (boxesOnCurrentPallet ==
            palletCapacity)
        {
            fullPallets++;
        }
        else
        {
            PalletStatistics lastPallet(
                palletId,
                boxesOnCurrentPallet *
                    boxVolume,
                palletVolume);

            result.getStatistics()
                .setLastPalletStatistics(
                    lastPallet);
        }

        palletId++;
    }

    Statistics& statistics =
        result.getStatistics();

    statistics.setTotalBoxes(
        boxesPlaced);

    statistics.setFullPallets(
        fullPallets);

    return result;
}