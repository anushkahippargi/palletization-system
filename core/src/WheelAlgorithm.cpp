#include "WheelAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

static constexpr double EPSILON = 0.000001;


// ============================================================
// Calculate the number of complete boxes that fit along
// one pallet dimension.
// ============================================================

int WheelAlgorithm::calculateBoxesAlong(
    double palletDimension,
    double boxDimension) const
{
    if (palletDimension <= 0.0 ||
        boxDimension <= 0.0)
    {
        return 0;
    }

    return static_cast<int>(
        floor(
            (palletDimension + EPSILON) /
            boxDimension
        )
    );
}


// ============================================================
// Evaluate one Wheel candidate.
//
// The supervisor's specification describes two variations:
//
//     1. Number of items along pallet L
//     2. Number of items along pallet W
//
// We use these two values to determine the dimensions of
// the outer Wheel pattern.
// ============================================================

WheelAlgorithm::Candidate
WheelAlgorithm::evaluateCandidate(
    const Pallet& pallet,
    const Orientation& orientation,
    int boxesAlongLength,
    int boxesAlongWidth) const
{
    Candidate candidate;

    candidate.boxesAlongLength =
        boxesAlongLength;

    candidate.boxesAlongWidth =
        boxesAlongWidth;


    if (boxesAlongLength < 2 ||
        boxesAlongWidth < 2)
    {
        return candidate;
    }


    // --------------------------------------------------------
    // The complete wheel occupies the selected rectangular
    // footprint.
    // --------------------------------------------------------

    candidate.usedLength =
        boxesAlongLength *
        orientation.length;

    candidate.usedWidth =
        boxesAlongWidth *
        orientation.width;


    // --------------------------------------------------------
    // Check that the complete footprint fits on the pallet.
    // --------------------------------------------------------

    if (candidate.usedLength >
            pallet.getLength() + EPSILON ||
        candidate.usedWidth >
            pallet.getWidth() + EPSILON)
    {
        return candidate;
    }


    // --------------------------------------------------------
    // Number of boxes in the outer wheel.
    //
    // A rectangular wheel is a perimeter:
    //
    //     top    = boxesAlongLength
    //     bottom = boxesAlongLength
    //     left   = boxesAlongWidth - 2
    //     right  = boxesAlongWidth - 2
    //
    // The corner boxes belong to the top/bottom sides and
    // therefore are not counted twice.
    // --------------------------------------------------------

    int top =
        boxesAlongLength;

    int bottom =
        boxesAlongLength;

    int left =
        boxesAlongWidth - 2;

    int right =
        boxesAlongWidth - 2;


    candidate.boxesPerLayer =
        top +
        bottom +
        left +
        right;


    if (candidate.boxesPerLayer <= 0)
    {
        return candidate;
    }


    // --------------------------------------------------------
    // Calculate unused pallet area.
    // --------------------------------------------------------

    double palletArea =
        pallet.getLength() *
        pallet.getWidth();

    double usedArea =
        candidate.usedLength *
        candidate.usedWidth;

    candidate.unusedArea =
        palletArea -
        usedArea;


    if (candidate.unusedArea < 0.0 &&
        candidate.unusedArea > -EPSILON)
    {
        candidate.unusedArea = 0.0;
    }


    candidate.orientation =
        orientation;

    candidate.valid = true;

    return candidate;
}


// ============================================================
// Add one placement to the result.
// ============================================================

void WheelAlgorithm::addPlacement(
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
        z
    );

    pose.setRotationZ(
        rotationZ
    );


    Placement placement(
        boxId,
        palletId,
        pose
    );


    result.getPlacements()
        .push_back(
            placement
        );
}


// ============================================================
// MAIN WHEEL ALGORITHM
// ============================================================

PalletizationResult WheelAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;


    // ========================================================
    // VALIDATION
    // ========================================================

    if (quantity <= 0)
    {
        result.getStatistics()
            .setTotalBoxes(0);

        result.getStatistics()
            .setFullPallets(0);

        return result;
    }


    const double palletLength =
        pallet.getLength();

    const double palletWidth =
        pallet.getWidth();

    const double palletHeight =
        pallet.getHeight();


    const double boxLength =
        box.getLength();

    const double boxWidth =
        box.getWidth();

    const double boxHeight =
        box.getHeight();


    if (palletLength <= 0.0 ||
        palletWidth <= 0.0 ||
        palletHeight <= 0.0 ||
        boxLength <= 0.0 ||
        boxWidth <= 0.0 ||
        boxHeight <= 0.0)
    {
        return result;
    }


    // ========================================================
    // TWO FLOOR ORIENTATIONS
    //
    // Normal:
    //
    //      L x W x H
    //
    // Rotated:
    //
    //      W x L x H
    // ========================================================

    Orientation normalOrientation
    {
        boxLength,
        boxWidth,
        boxHeight,
        0.0
    };


    Orientation rotatedOrientation
    {
        boxWidth,
        boxLength,
        boxHeight,
        90.0
    };


    // ========================================================
    // FIND BEST WHEEL CANDIDATE
    // ========================================================

    Candidate bestCandidate;

    bool hasCandidate = false;


    auto isBetterCandidate =
        [](const Candidate& candidate,
           const Candidate& best) -> bool
    {
        // ----------------------------------------------------
        // Rule 1:
        // More boxes in the Wheel is better.
        // ----------------------------------------------------

        if (candidate.boxesPerLayer !=
            best.boxesPerLayer)
        {
            return candidate.boxesPerLayer >
                   best.boxesPerLayer;
        }


        // ----------------------------------------------------
        // Rule 2:
        // If equal, prefer the pattern with less unused area.
        // ----------------------------------------------------

        if (fabs(
                candidate.unusedArea -
                best.unusedArea
            ) > EPSILON)
        {
            return candidate.unusedArea <
                   best.unusedArea;
        }


        return false;
    };


    // ========================================================
    // EVALUATE BOTH FLOOR ORIENTATIONS
    // ========================================================

    Orientation orientations[2] =
    {
        normalOrientation,
        rotatedOrientation
    };


    for (const Orientation& orientation :
         orientations)
    {
        int maximumLength =
            calculateBoxesAlong(
                palletLength,
                orientation.length
            );


        int maximumWidth =
            calculateBoxesAlong(
                palletWidth,
                orientation.width
            );


        // ----------------------------------------------------
        // Wheel variation 1:
        // number of boxes along pallet L
        // ----------------------------------------------------

        for (int boxesAlongLength = 2;
             boxesAlongLength <= maximumLength;
             ++boxesAlongLength)
        {
            // ------------------------------------------------
            // Wheel variation 2:
            // number of boxes along pallet W
            // ------------------------------------------------

            for (int boxesAlongWidth = 2;
                 boxesAlongWidth <= maximumWidth;
                 ++boxesAlongWidth)
            {
                Candidate candidate =
                    evaluateCandidate(
                        pallet,
                        orientation,
                        boxesAlongLength,
                        boxesAlongWidth
                    );


                if (!candidate.valid)
                {
                    continue;
                }


                if (!hasCandidate ||
                    isBetterCandidate(
                        candidate,
                        bestCandidate
                    ))
                {
                    bestCandidate =
                        candidate;

                    hasCandidate = true;
                }
            }
        }
    }


    // ========================================================
    // NO VALID WHEEL
    // ========================================================

    if (!hasCandidate)
    {
        return result;
    }


    // ========================================================
    // DISPLAY SELECTED WHEEL
    // ========================================================

    cout << "\n========================================"
         << endl;

    cout << "WHEEL ALGORITHM"
         << endl;

    cout << "========================================"
         << endl;


    cout << "Selected orientation: "
         << bestCandidate.orientation.length
         << " x "
         << bestCandidate.orientation.width
         << " x "
         << bestCandidate.orientation.height
         << endl;


    cout << "Boxes along pallet L: "
         << bestCandidate.boxesAlongLength
         << endl;


    cout << "Boxes along pallet W: "
         << bestCandidate.boxesAlongWidth
         << endl;


    cout << "Boxes in wheel layer: "
         << bestCandidate.boxesPerLayer
         << endl;


    cout << "Used length: "
         << bestCandidate.usedLength
         << endl;


    cout << "Used width: "
         << bestCandidate.usedWidth
         << endl;


    cout << "Unused area: "
         << bestCandidate.unusedArea
         << endl;


    // ========================================================
    // VERTICAL LAYERS
    // ========================================================

    int layers =
        calculateBoxesAlong(
            palletHeight,
            boxHeight
        );


    if (layers <= 0)
    {
        return result;
    }


    int boxesPerPallet =
        bestCandidate.boxesPerLayer *
        layers;


    cout << "Layers: "
         << layers
         << endl;


    cout << "Boxes per pallet: "
         << boxesPerPallet
         << endl;


    // ========================================================
    // VOLUMES
    // ========================================================

    const double boxVolume =
        boxLength *
        boxWidth *
        boxHeight;


    const double palletVolume =
        palletLength *
        palletWidth *
        palletHeight;


    // ========================================================
    // PALLETIZATION
    // ========================================================

    int boxesPlaced = 0;

    int fullPallets = 0;

    int palletId = 1;


    while (boxesPlaced < quantity)
    {
        int boxesOnCurrentPallet = 0;


        // ====================================================
        // EACH LAYER
        // ====================================================

        for (int layer = 0;
             layer < layers &&
             boxesPlaced < quantity;
             ++layer)
        {
            const double z =
                layer *
                boxHeight;


            // ------------------------------------------------
            // Alternate layers.
            //
            // Layer 0 = normal wheel
            // Layer 1 = flipped wheel
            // Layer 2 = normal wheel
            // Layer 3 = flipped wheel
            //
            // This creates the required brick-effect.
            // ------------------------------------------------

            bool flipped =
                (layer % 2 == 1);


            // ------------------------------------------------
            // Center the complete wheel on the pallet.
            // ------------------------------------------------

            double offsetX =
                (palletLength -
                 bestCandidate.usedLength) /
                2.0;


            double offsetY =
                (palletWidth -
                 bestCandidate.usedWidth) /
                2.0;


            if (offsetX < 0.0 &&
                offsetX > -EPSILON)
            {
                offsetX = 0.0;
            }


            if (offsetY < 0.0 &&
                offsetY > -EPSILON)
            {
                offsetY = 0.0;
            }


            const double L =
                bestCandidate.orientation.length;


            const double W =
                bestCandidate.orientation.width;


            const int nL =
                bestCandidate.boxesAlongLength;


            const int nW =
                bestCandidate.boxesAlongWidth;


            // =================================================
            // WHEEL
            //
            // Top side
            // =================================================

            for (int i = 0;
                 i < nL &&
                 boxesPlaced < quantity;
                 ++i)
            {
                double x =
                    offsetX +
                    i * L;


                double y =
                    offsetY;


                double rotation =
                    bestCandidate.orientation.rotationZ;


                // Flip the whole layer around the pallet
                // center.
                if (flipped)
                {
                    x =
                        palletLength -
                        x -
                        L;

                    y =
                        palletWidth -
                        y -
                        W;

                    rotation += 180.0;
                }


                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    x,
                    y,
                    z,
                    rotation
                );


                ++boxesPlaced;
                ++boxesOnCurrentPallet;
            }


            // =================================================
            // BOTTOM SIDE
            //
            // We don't add this if the top side has already
            // filled the requested quantity.
            // =================================================

            for (int i = 0;
                 i < nL &&
                 boxesPlaced < quantity;
                 ++i)
            {
                double x =
                    offsetX +
                    i * L;


                double y =
                    offsetY +
                    (nW - 1) * W;


                double rotation =
                    bestCandidate.orientation.rotationZ;


                if (flipped)
                {
                    x =
                        palletLength -
                        x -
                        L;

                    y =
                        palletWidth -
                        y -
                        W;

                    rotation += 180.0;
                }


                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    x,
                    y,
                    z,
                    rotation
                );


                ++boxesPlaced;
                ++boxesOnCurrentPallet;
            }


            // =================================================
            // LEFT SIDE
            //
            // Start from row 1 and stop before the bottom
            // corner, because the corners were already placed.
            // =================================================

            for (int j = 1;
                 j < nW - 1 &&
                 boxesPlaced < quantity;
                 ++j)
            {
                double x =
                    offsetX;


                double y =
                    offsetY +
                    j * W;


                // Side boxes need the rotated orientation.
                double rotation =
                    bestCandidate.orientation.rotationZ +
                    90.0;


                if (flipped)
                {
                    x =
                        palletLength -
                        x -
                        W;

                    y =
                        palletWidth -
                        y -
                        L;

                    rotation += 180.0;
                }


                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    x,
                    y,
                    z,
                    rotation
                );


                ++boxesPlaced;
                ++boxesOnCurrentPallet;
            }


            // =================================================
            // RIGHT SIDE
            //
            // Again, corners are excluded.
            // =================================================

            for (int j = 1;
                 j < nW - 1 &&
                 boxesPlaced < quantity;
                 ++j)
            {
                double x =
                    offsetX +
                    (nL - 1) * L;


                double y =
                    offsetY +
                    j * W;


                double rotation =
                    bestCandidate.orientation.rotationZ +
                    90.0;


                if (flipped)
                {
                    x =
                        palletLength -
                        x -
                        W;

                    y =
                        palletWidth -
                        y -
                        L;

                    rotation += 180.0;
                }


                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    x,
                    y,
                    z,
                    rotation
                );


                ++boxesPlaced;
                ++boxesOnCurrentPallet;
            }
        }


        // ====================================================
        // STATISTICS
        // ====================================================

        if (boxesOnCurrentPallet ==
            boxesPerPallet)
        {
            ++fullPallets;
        }
        else if (boxesOnCurrentPallet > 0)
        {
            PalletStatistics lastPallet(
                palletId,
                boxesOnCurrentPallet *
                    boxVolume,
                palletVolume
            );


            result.getStatistics()
                .setLastPalletStatistics(
                    lastPallet
                );
        }


        // ----------------------------------------------------
        // Safety check.
        // ----------------------------------------------------

        if (boxesOnCurrentPallet == 0)
        {
            break;
        }


        ++palletId;
    }


    // ========================================================
    // FINAL STATISTICS
    // ========================================================

    result.getStatistics()
        .setTotalBoxes(
            boxesPlaced
        );


    result.getStatistics()
        .setFullPallets(
            fullPallets
        );


    cout << "\n----------------------------------------"
         << endl;

    cout << "WHEEL RESULT"
         << endl;

    cout << "----------------------------------------"
         << endl;

    cout << "Boxes requested: "
         << quantity
         << endl;

    cout << "Boxes placed: "
         << boxesPlaced
         << endl;

    cout << "Full pallets: "
         << fullPallets
         << endl;

    cout << "----------------------------------------"
         << endl;


    return result;
}