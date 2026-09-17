#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "BricksAlgorithm.h"

using namespace std;


int BricksAlgorithm::calculateBoxesAlong(
    double palletDimension,
    double boxDimension) const
{
    if (boxDimension <= 0)
        return 0;

    return static_cast<int>(
        palletDimension / boxDimension);
}


BricksAlgorithm::Candidate
BricksAlgorithm::evaluateCandidate(
    const Pallet& pallet,
    const Orientation& orientation,
    int boxesAlongLength,
    int boxesAlongWidth) const
{
    Candidate candidate;

    if (boxesAlongLength <= 0 ||
        boxesAlongWidth <= 0)
    {
        return candidate;
    }

    double usedLength =
        boxesAlongLength *
        orientation.length;

    double usedWidth =
        boxesAlongWidth *
        orientation.width;

    if (usedLength > pallet.getLength() ||
        usedWidth > pallet.getWidth())
    {
        return candidate;
    }

    candidate.valid = true;

    candidate.boxesAlongLength =
        boxesAlongLength;

    candidate.boxesAlongWidth =
        boxesAlongWidth;

    candidate.boxesPerLayer =
        boxesAlongLength *
        boxesAlongWidth;

    candidate.usedLength =
        usedLength;

    candidate.usedWidth =
        usedWidth;

    candidate.unusedArea =
        (pallet.getLength() *
         pallet.getWidth()) -
        (usedLength *
         usedWidth);

    candidate.orientation =
        orientation;

    return candidate;
}


void BricksAlgorithm::addPlacement(
    PalletizationResult& result,
    int boxId,
    int palletId,
    double x,
    double y,
    double z,
    double rotationZ) const
{
    Matrix4x4 pose;

    pose.setTranslation(
        x,
        y,
        z);

    /*
     * Bricks uses rotation around the Z axis
     * to reverse the direction of alternate
     * rows/layers.
     */
    pose.setRotationZ(
        rotationZ);

    Placement placement(
        boxId,
        palletId,
        pose);

    result.getPlacements()
        .push_back(placement);
}


PalletizationResult
BricksAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;

    if (quantity <= 0)
        return result;


    /*
     * --------------------------------------------------
     * TWO HORIZONTAL ORIENTATIONS
     * --------------------------------------------------
     *
     * Orientation 1:
     * Box Length -> Pallet Length
     * Box Width  -> Pallet Width
     *
     * Orientation 2:
     * Box Width  -> Pallet Length
     * Box Length -> Pallet Width
     *
     * We select the orientation that gives the
     * highest number of boxes per layer.
     */

    Orientation normal =
    {
        box.getLength(),
        box.getWidth(),
        box.getHeight(),
        0.0
    };

    Orientation rotated =
    {
        box.getWidth(),
        box.getLength(),
        box.getHeight(),
        90.0
    };


    Candidate candidates[2];


    candidates[0] =
        evaluateCandidate(
            pallet,
            normal,
            calculateBoxesAlong(
                pallet.getLength(),
                normal.length),
            calculateBoxesAlong(
                pallet.getWidth(),
                normal.width));


    candidates[1] =
        evaluateCandidate(
            pallet,
            rotated,
            calculateBoxesAlong(
                pallet.getLength(),
                rotated.length),
            calculateBoxesAlong(
                pallet.getWidth(),
                rotated.width));


    Candidate best;

    for (const Candidate& candidate : candidates)
    {
        if (!candidate.valid)
            continue;

        if (!best.valid)
        {
            best = candidate;
            continue;
        }

        /*
         * Rule 1:
         * Prefer the candidate with more boxes
         * per layer.
         */
        if (candidate.boxesPerLayer >
            best.boxesPerLayer)
        {
            best = candidate;
        }

        /*
         * Rule 2:
         * If capacity is equal, prefer the candidate
         * that leaves less unused pallet area.
         */
        else if (
            candidate.boxesPerLayer ==
                best.boxesPerLayer &&
            candidate.unusedArea <
                best.unusedArea)
        {
            best = candidate;
        }
    }


    if (!best.valid)
        return result;


    /*
     * --------------------------------------------------
     * VERTICAL CAPACITY
     * --------------------------------------------------
     */

    int boxesAlongZ =
        static_cast<int>(
            pallet.getHeight() /
            best.orientation.height);


    if (boxesAlongZ <= 0)
        return result;


    int palletCapacity =
        best.boxesPerLayer *
        boxesAlongZ;


    /*
     * --------------------------------------------------
     * STATISTICS VALUES
     * --------------------------------------------------
     */

    double boxVolume =
        box.getLength() *
        box.getWidth() *
        box.getHeight();

    double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();


    /*
     * --------------------------------------------------
     * CENTER THE PATTERN ON THE PALLET
     * --------------------------------------------------
     */

    double offsetX =
        (pallet.getLength() -
         best.usedLength) / 2.0;

    double offsetY =
        (pallet.getWidth() -
         best.usedWidth) / 2.0;


    cout << "\nBRICKS ALGORITHM"
         << endl;

    cout << "Selected Orientation: "
         << best.orientation.length
         << " x "
         << best.orientation.width
         << " x "
         << best.orientation.height
         << endl;

    cout << "Rotation X: 0 degrees"
         << endl;

    cout << "Rotation Y: 0 degrees"
         << endl;

    cout << "Base Rotation Z: "
         << best.orientation.rotationZ
         << " degrees"
         << endl;

    cout << "Boxes along X: "
         << best.boxesAlongLength
         << endl;

    cout << "Boxes along Y: "
         << best.boxesAlongWidth
         << endl;

    cout << "Boxes along Z: "
         << boxesAlongZ
         << endl;

    cout << "Boxes per Layer: "
         << best.boxesPerLayer
         << endl;

    cout << "Pallet Capacity: "
         << palletCapacity
         << endl;


    /*
     * --------------------------------------------------
     * PLACE BOXES
     * --------------------------------------------------
     */

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
            /*
             * Every alternate layer is reversed.
             *
             * This creates the brick-style alternating
             * stacking pattern while keeping every box
             * completely inside the pallet.
             */

            bool reverseLayer =
                (zIndex % 2 == 1);


            double z =
                zIndex *
                best.orientation.height;


            for (int yIndex = 0;
                 yIndex < best.boxesAlongWidth &&
                 boxesPlaced < quantity;
                 yIndex++)
            {
                /*
                 * Alternate rows are also traversed
                 * in the opposite direction.
                 *
                 * This gives the visualization the
                 * characteristic brick arrangement.
                 */

                bool reverseRow =
                    ((yIndex + zIndex) % 2 == 1);


                for (int xIndex = 0;
                     xIndex < best.boxesAlongLength &&
                     boxesPlaced < quantity;
                     xIndex++)
                {
                    int actualXIndex;

                    if (reverseRow)
                    {
                        actualXIndex =
                            best.boxesAlongLength -
                            1 -
                            xIndex;
                    }
                    else
                    {
                        actualXIndex =
                            xIndex;
                    }


                    double x =
                        offsetX +
                        actualXIndex *
                        best.orientation.length;


                    double y =
                        offsetY +
                        yIndex *
                        best.orientation.width;


                    /*
                     * When the layer is reversed,
                     * rotate the boxes by 180 degrees.
                     *
                     * The footprint remains the same,
                     * but the orientation of the layer
                     * alternates.
                     */

                    double rotationZ =
                        best.orientation.rotationZ;


                    if (reverseLayer)
                    {
                        rotationZ += 180.0;
                    }


                    /*
                     * Keep rotation within 0-360 degrees.
                     */

                    if (rotationZ >= 360.0)
                    {
                        rotationZ -= 360.0;
                    }


                    addPlacement(
                        result,
                        boxesPlaced + 1,
                        palletId,
                        x,
                        y,
                        z,
                        rotationZ);


                    boxesPlaced++;

                    boxesOnCurrentPallet++;
                }
            }
        }


        /*
         * --------------------------------------------------
         * PALLET STATISTICS
         * --------------------------------------------------
         */

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


        /*
         * Safety check.
         */

        if (boxesOnCurrentPallet == 0)
            break;
    }


    /*
     * --------------------------------------------------
     * FINAL STATISTICS
     * --------------------------------------------------
     */

    Statistics& statistics =
        result.getStatistics();


    statistics.setTotalBoxes(
        boxesPlaced);


    statistics.setFullPallets(
        fullPallets);


    return result;
}