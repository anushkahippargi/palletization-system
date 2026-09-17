#include "RowsColsAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;


// ============================================================
// CONSTANT
// ============================================================

static constexpr double EPSILON = 0.000001;


// ============================================================
// Calculate how many complete boxes fit along one dimension
// ============================================================

int RowsColsAlgorithm::calculateBoxesAlong(
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
// Generate one candidate pattern
//
// The pattern contains two rectangular sections:
//
//     Section 1 -> normal box orientation
//     Section 2 -> rotated box orientation
//
// They are separated either:
//
//     Along pallet Length
//     Along pallet Width
//
// This guarantees that the two sections cannot overlap.
// ============================================================

RowsColsAlgorithm::Candidate
RowsColsAlgorithm::generateCandidate(
    const Pallet& pallet,
    const Box& box,
    const Orientation& normalOrientation,
    const Orientation& rotatedOrientation,
    int normalColumns,
    int normalRows,
    int rotatedColumns,
    int rotatedRows,
    SplitDirection splitDirection) const
{
    Candidate candidate;

    candidate.splitDirection =
        splitDirection;

    candidate.normalColumns =
        normalColumns;

    candidate.normalRows =
        normalRows;

    candidate.rotatedColumns =
        rotatedColumns;

    candidate.rotatedRows =
        rotatedRows;

    candidate.normalOrientation =
        normalOrientation;

    candidate.rotatedOrientation =
        rotatedOrientation;


    // --------------------------------------------------------
    // Capacity of one layer
    // --------------------------------------------------------

    candidate.capacity =
        (normalColumns * normalRows) +
        (rotatedColumns * rotatedRows);


    if (candidate.capacity <= 0)
    {
        return candidate;
    }


    // --------------------------------------------------------
    // Split along pallet LENGTH
    //
    // Example:
    //
    // ┌──────────────┬───────────┐
    // │              │           │
    // │   NORMAL     │  ROTATED  │
    // │              │           │
    // └──────────────┴───────────┘
    //
    // Normal section:
    //     length = columns * normal.length
    //     width  = rows    * normal.width
    //
    // Rotated section:
    //     length = columns * rotated.length
    //     width  = rows    * rotated.width
    // --------------------------------------------------------

    if (splitDirection ==
        SplitDirection::AlongLength)
    {
        double normalLength =
            normalColumns *
            normalOrientation.length;

        double rotatedLength =
            rotatedColumns *
            rotatedOrientation.length;

        double normalWidth =
            normalRows *
            normalOrientation.width;

        double rotatedWidth =
            rotatedRows *
            rotatedOrientation.width;


        candidate.usedLength =
            normalLength +
            rotatedLength;

        candidate.usedWidth =
            max(
                normalWidth,
                rotatedWidth
            );
    }


    // --------------------------------------------------------
    // Split along pallet WIDTH
    //
    // Example:
    //
    // ┌─────────────────────────┐
    // │        NORMAL           │
    // ├─────────────────────────┤
    // │        ROTATED          │
    // └─────────────────────────┘
    // --------------------------------------------------------

    else
    {
        double normalLength =
            normalColumns *
            normalOrientation.length;

        double rotatedLength =
            rotatedColumns *
            rotatedOrientation.length;

        double normalWidth =
            normalRows *
            normalOrientation.width;

        double rotatedWidth =
            rotatedRows *
            rotatedOrientation.width;


        candidate.usedLength =
            max(
                normalLength,
                rotatedLength
            );

        candidate.usedWidth =
            normalWidth +
            rotatedWidth;
    }


    // --------------------------------------------------------
    // Calculate unused pallet area
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


    return candidate;
}


// ============================================================
// MAIN ROWS + COLUMNS ALGORITHM
// ============================================================

PalletizationResult RowsColsAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;


    // ========================================================
    // BASIC VALIDATION
    // ========================================================

    if (quantity <= 0)
    {
        result.getStatistics()
            .setTotalBoxes(0);

        result.getStatistics()
            .setFullPallets(0);

        return result;
    }


    double palletLength =
        pallet.getLength();

    double palletWidth =
        pallet.getWidth();

    double palletHeight =
        pallet.getHeight();


    double boxLength =
        box.getLength();

    double boxWidth =
        box.getWidth();

    double boxHeight =
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
    //     L × W × H
    //
    // Rotated:
    //
    //     W × L × H
    //
    // Only the floor orientation is changed.
    // The box is not rotated vertically.
    // ========================================================

    Orientation normalOrientation
    {
        boxLength,
        boxWidth,
        boxHeight,

        0.0,
        0.0,
        0.0
    };


    Orientation rotatedOrientation
    {
        boxWidth,
        boxLength,
        boxHeight,

        0.0,
        0.0,
        90.0
    };


    // ========================================================
    // BEST CANDIDATE
    // ========================================================

    Candidate bestCandidate;

    bool hasCandidate = false;


    // ========================================================
    // CANDIDATE COMPARISON
    //
    // Priority:
    //
    // 1. Maximum boxes per layer
    // 2. Minimum unused pallet area
    // 3. Prefer fewer rotated boxes
    // ========================================================

    auto isBetterCandidate =
        [](const Candidate& candidate,
           const Candidate& best) -> bool
    {
        // ----------------------------------------------------
        // Rule 1:
        // More boxes is always better.
        // ----------------------------------------------------

        if (candidate.capacity !=
            best.capacity)
        {
            return candidate.capacity >
                   best.capacity;
        }


        // ----------------------------------------------------
        // Rule 2:
        // If capacity is equal, prefer the pattern
        // occupying less unused pallet area.
        // ----------------------------------------------------

        if (fabs(
                candidate.unusedArea -
                best.unusedArea
            ) > EPSILON)
        {
            return candidate.unusedArea <
                   best.unusedArea;
        }


        // ----------------------------------------------------
        // Rule 3:
        // If still tied, prefer fewer rotated boxes.
        // ----------------------------------------------------

        int candidateRotated =
            candidate.rotatedColumns *
            candidate.rotatedRows;

        int bestRotated =
            best.rotatedColumns *
            best.rotatedRows;


        return candidateRotated <
               bestRotated;
    };


    // ========================================================
    // MAXIMUM POSSIBLE DIMENSIONS
    // ========================================================

    int maxNormalColumns =
        calculateBoxesAlong(
            palletLength,
            normalOrientation.length
        );

    int maxNormalRows =
        calculateBoxesAlong(
            palletWidth,
            normalOrientation.width
        );

    int maxRotatedColumns =
        calculateBoxesAlong(
            palletLength,
            rotatedOrientation.length
        );

    int maxRotatedRows =
        calculateBoxesAlong(
            palletWidth,
            rotatedOrientation.width
        );


    // ========================================================
    // SHOW BASIC INFORMATION
    // ========================================================

    cout << "\n========================================"
         << endl;

    cout << "ROWS + COLUMNS ALGORITHM"
         << endl;

    cout << "========================================"
         << endl;

    cout << "Normal orientation : "
         << normalOrientation.length
         << " x "
         << normalOrientation.width
         << " x "
         << normalOrientation.height
         << endl;

    cout << "Rotated orientation: "
         << rotatedOrientation.length
         << " x "
         << rotatedOrientation.width
         << " x "
         << rotatedOrientation.height
         << endl;

    cout << "Pallet              : "
         << palletLength
         << " x "
         << palletWidth
         << " x "
         << palletHeight
         << endl;


    // ========================================================
    // EVALUATE SPLIT ALONG LENGTH
    //
    // We enumerate the number of columns in each section.
    //
    // Once the section widths are known, the maximum possible
    // rows are calculated automatically.
    //
    // This avoids generating overlapping or impossible
    // combinations.
    // ========================================================

    for (int normalColumns = 0;
         normalColumns <= maxNormalColumns;
         ++normalColumns)
    {
        double normalSectionLength =
            normalColumns *
            normalOrientation.length;


        double remainingLength =
            palletLength -
            normalSectionLength;


        if (remainingLength < -EPSILON)
        {
            continue;
        }


        int maximumRotatedColumns =
            calculateBoxesAlong(
                remainingLength,
                rotatedOrientation.length
            );


        for (int rotatedColumns = 0;
             rotatedColumns <= maximumRotatedColumns;
             ++rotatedColumns)
        {
            // ------------------------------------------------
            // Normal section rows
            // ------------------------------------------------

            int normalRows = 0;

            if (normalColumns > 0)
            {
                normalRows =
                    maxNormalRows;
            }


            // ------------------------------------------------
            // Rotated section rows
            // ------------------------------------------------

            int rotatedRows = 0;

            if (rotatedColumns > 0)
            {
                rotatedRows =
                    maxRotatedRows;
            }


            // ------------------------------------------------
            // At least one section must exist.
            // ------------------------------------------------

            if ((normalColumns == 0 ||
                 normalRows == 0) &&
                (rotatedColumns == 0 ||
                 rotatedRows == 0))
            {
                continue;
            }


            Candidate candidate =
                generateCandidate(
                    pallet,
                    box,
                    normalOrientation,
                    rotatedOrientation,
                    normalColumns,
                    normalRows,
                    rotatedColumns,
                    rotatedRows,
                    SplitDirection::AlongLength
                );


            if (candidate.capacity <= 0)
            {
                continue;
            }


            // ------------------------------------------------
            // Check complete pattern fits on pallet.
            // ------------------------------------------------

            if (candidate.usedLength >
                    palletLength + EPSILON ||
                candidate.usedWidth >
                    palletWidth + EPSILON)
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


    // ========================================================
    // EVALUATE SPLIT ALONG WIDTH
    // ========================================================

    for (int normalRows = 0;
         normalRows <= maxNormalRows;
         ++normalRows)
    {
        double normalSectionWidth =
            normalRows *
            normalOrientation.width;


        double remainingWidth =
            palletWidth -
            normalSectionWidth;


        if (remainingWidth < -EPSILON)
        {
            continue;
        }


        int maximumRotatedRows =
            calculateBoxesAlong(
                remainingWidth,
                rotatedOrientation.width
            );


        for (int rotatedRows = 0;
             rotatedRows <= maximumRotatedRows;
             ++rotatedRows)
        {
            // ------------------------------------------------
            // Normal section columns
            // ------------------------------------------------

            int normalColumns = 0;

            if (normalRows > 0)
            {
                normalColumns =
                    maxNormalColumns;
            }


            // ------------------------------------------------
            // Rotated section columns
            // ------------------------------------------------

            int rotatedColumns = 0;

            if (rotatedRows > 0)
            {
                rotatedColumns =
                    maxRotatedColumns;
            }


            if ((normalColumns == 0 ||
                 normalRows == 0) &&
                (rotatedColumns == 0 ||
                 rotatedRows == 0))
            {
                continue;
            }


            Candidate candidate =
                generateCandidate(
                    pallet,
                    box,
                    normalOrientation,
                    rotatedOrientation,
                    normalColumns,
                    normalRows,
                    rotatedColumns,
                    rotatedRows,
                    SplitDirection::AlongWidth
                );


            if (candidate.capacity <= 0)
            {
                continue;
            }


            // ------------------------------------------------
            // Check complete pattern fits.
            // ------------------------------------------------

            if (candidate.usedLength >
                    palletLength + EPSILON ||
                candidate.usedWidth >
                    palletWidth + EPSILON)
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


    // ========================================================
    // NO VALID PATTERN
    // ========================================================

    if (!hasCandidate)
    {
        cout << "\nNo valid Rows + Columns pattern found."
             << endl;

        return result;
    }


    // ========================================================
    // DISPLAY SELECTED PATTERN
    // ========================================================

    cout << "\n----------------------------------------"
         << endl;

    cout << "SELECTED ROWS + COLUMNS PATTERN"
         << endl;

    cout << "----------------------------------------"
         << endl;


    if (bestCandidate.splitDirection ==
        SplitDirection::AlongLength)
    {
        cout << "Split direction: PALLET LENGTH"
             << endl;
    }
    else
    {
        cout << "Split direction: PALLET WIDTH"
             << endl;
    }


    cout << "Normal section:"
         << endl;

    cout << "  Columns: "
         << bestCandidate.normalColumns
         << endl;

    cout << "  Rows   : "
         << bestCandidate.normalRows
         << endl;


    cout << "Rotated section:"
         << endl;

    cout << "  Columns: "
         << bestCandidate.rotatedColumns
         << endl;

    cout << "  Rows   : "
         << bestCandidate.rotatedRows
         << endl;


    cout << "Boxes per layer: "
         << bestCandidate.capacity
         << endl;


    cout << "Used length: "
         << bestCandidate.usedLength
         << endl;

    cout << "Used width : "
         << bestCandidate.usedWidth
         << endl;


    cout << "Unused area: "
         << bestCandidate.unusedArea
         << endl;


    // ========================================================
    // PALLET CAPACITY
    //
    // Number of vertical layers.
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
        bestCandidate.capacity *
        layers;


    if (boxesPerPallet <= 0)
    {
        return result;
    }


    cout << "Layers: "
         << layers
         << endl;

    cout << "Boxes per pallet: "
         << boxesPerPallet
         << endl;


    // ========================================================
    // PALLETIZATION
    // ========================================================

    int boxesPlaced = 0;

    int fullPallets = 0;

    int palletId = 1;


    double boxVolume =
        boxLength *
        boxWidth *
        boxHeight;


    double palletVolume =
        palletLength *
        palletWidth *
        palletHeight;


    while (boxesPlaced < quantity)
    {
        int boxesOnCurrentPallet = 0;


        // ====================================================
        // EACH VERTICAL LAYER
        // ====================================================

        for (int layer = 0;
             layer < layers &&
             boxesPlaced < quantity;
             ++layer)
        {
            double z =
                layer *
                boxHeight;


            // Alternate the pattern between layers.
            //
            // Layer 0 -> normal
            // Layer 1 -> flipped
            // Layer 2 -> normal
            // Layer 3 -> flipped
            //

            bool flipped =
                (layer % 2 == 1);


            // =================================================
            // Calculate the complete pattern dimensions.
            // =================================================

            double patternLength =
                bestCandidate.usedLength;

            double patternWidth =
                bestCandidate.usedWidth;


            double lengthOffset =
                (palletLength -
                 patternLength) / 2.0;


            double widthOffset =
                (palletWidth -
                 patternWidth) / 2.0;


            // Prevent tiny negative values caused by
            // floating-point calculations.

            if (lengthOffset < 0.0 &&
                lengthOffset > -EPSILON)
            {
                lengthOffset = 0.0;
            }

            if (widthOffset < 0.0 &&
                widthOffset > -EPSILON)
            {
                widthOffset = 0.0;
            }


            // =================================================
            // SPLIT ALONG LENGTH
            //
            // Normal section is on the left.
            // Rotated section is on the right.
            // =================================================

            if (bestCandidate.splitDirection ==
                SplitDirection::AlongLength)
            {
                // ---------------------------------------------
                // NORMAL SECTION
                // ---------------------------------------------

                for (int row = 0;
                     row < bestCandidate.normalRows &&
                     boxesPlaced < quantity;
                     ++row)
                {
                    for (int column = 0;
                         column < bestCandidate.normalColumns &&
                         boxesPlaced < quantity;
                         ++column)
                    {
                        double x =
                            lengthOffset +
                            column *
                            bestCandidate.normalOrientation.length;

                        double y =
                            widthOffset +
                            row *
                            bestCandidate.normalOrientation.width;


                        double footprintLength =
                            bestCandidate.normalOrientation.length;

                        double footprintWidth =
                            bestCandidate.normalOrientation.width;


                        double rotation =
                            bestCandidate.normalOrientation.rotationZ;


                        // -------------------------------------
                        // Flip the complete pattern on
                        // alternating layers.
                        // -------------------------------------

                        if (flipped)
                        {
                            x =
                                palletLength -
                                x -
                                footprintLength;

                            y =
                                palletWidth -
                                y -
                                footprintWidth;

                            rotation += 180.0;
                        }


                        Matrix4x4 pose;

                        pose.setTranslation(
                            x,
                            y,
                            z
                        );

                        pose.setRotationZ(
                            rotation
                        );


                        Placement placement(
                            boxesPlaced + 1,
                            palletId,
                            pose
                        );


                        result.getPlacements()
                            .push_back(
                                placement
                            );


                        ++boxesPlaced;

                        ++boxesOnCurrentPallet;
                    }
                }


                // ---------------------------------------------
                // ROTATED SECTION
                // ---------------------------------------------

                double rotatedStartX =
                    lengthOffset +
                    bestCandidate.normalColumns *
                    bestCandidate.normalOrientation.length;


                double rotatedStartY =
                    widthOffset;


                for (int row = 0;
                     row < bestCandidate.rotatedRows &&
                     boxesPlaced < quantity;
                     ++row)
                {
                    for (int column = 0;
                         column < bestCandidate.rotatedColumns &&
                         boxesPlaced < quantity;
                         ++column)
                    {
                        double x =
                            rotatedStartX +
                            column *
                            bestCandidate.rotatedOrientation.length;

                        double y =
                            rotatedStartY +
                            row *
                            bestCandidate.rotatedOrientation.width;


                        double footprintLength =
                            bestCandidate.rotatedOrientation.length;

                        double footprintWidth =
                            bestCandidate.rotatedOrientation.width;


                        double rotation =
                            bestCandidate.rotatedOrientation.rotationZ;


                        if (flipped)
                        {
                            x =
                                palletLength -
                                x -
                                footprintLength;

                            y =
                                palletWidth -
                                y -
                                footprintWidth;

                            rotation += 180.0;
                        }


                        Matrix4x4 pose;

                        pose.setTranslation(
                            x,
                            y,
                            z
                        );

                        pose.setRotationZ(
                            rotation
                        );


                        Placement placement(
                            boxesPlaced + 1,
                            palletId,
                            pose
                        );


                        result.getPlacements()
                            .push_back(
                                placement
                            );


                        ++boxesPlaced;

                        ++boxesOnCurrentPallet;
                    }
                }
            }


            // =================================================
            // SPLIT ALONG WIDTH
            //
            // Normal section is at the bottom.
            // Rotated section is above it.
            // =================================================

            else
            {
                // ---------------------------------------------
                // NORMAL SECTION
                // ---------------------------------------------

                for (int row = 0;
                     row < bestCandidate.normalRows &&
                     boxesPlaced < quantity;
                     ++row)
                {
                    for (int column = 0;
                         column < bestCandidate.normalColumns &&
                         boxesPlaced < quantity;
                         ++column)
                    {
                        double x =
                            lengthOffset +
                            column *
                            bestCandidate.normalOrientation.length;

                        double y =
                            widthOffset +
                            row *
                            bestCandidate.normalOrientation.width;


                        double footprintLength =
                            bestCandidate.normalOrientation.length;

                        double footprintWidth =
                            bestCandidate.normalOrientation.width;


                        double rotation =
                            bestCandidate.normalOrientation.rotationZ;


                        if (flipped)
                        {
                            x =
                                palletLength -
                                x -
                                footprintLength;

                            y =
                                palletWidth -
                                y -
                                footprintWidth;

                            rotation += 180.0;
                        }


                        Matrix4x4 pose;

                        pose.setTranslation(
                            x,
                            y,
                            z
                        );

                        pose.setRotationZ(
                            rotation
                        );


                        Placement placement(
                            boxesPlaced + 1,
                            palletId,
                            pose
                        );


                        result.getPlacements()
                            .push_back(
                                placement
                            );


                        ++boxesPlaced;

                        ++boxesOnCurrentPallet;
                    }
                }


                // ---------------------------------------------
                // ROTATED SECTION
                // ---------------------------------------------

                double rotatedStartX =
                    lengthOffset;


                double rotatedStartY =
                    widthOffset +
                    bestCandidate.normalRows *
                    bestCandidate.normalOrientation.width;


                for (int row = 0;
                     row < bestCandidate.rotatedRows &&
                     boxesPlaced < quantity;
                     ++row)
                {
                    for (int column = 0;
                         column < bestCandidate.rotatedColumns &&
                         boxesPlaced < quantity;
                         ++column)
                    {
                        double x =
                            rotatedStartX +
                            column *
                            bestCandidate.rotatedOrientation.length;

                        double y =
                            rotatedStartY +
                            row *
                            bestCandidate.rotatedOrientation.width;


                        double footprintLength =
                            bestCandidate.rotatedOrientation.length;

                        double footprintWidth =
                            bestCandidate.rotatedOrientation.width;


                        double rotation =
                            bestCandidate.rotatedOrientation.rotationZ;


                        if (flipped)
                        {
                            x =
                                palletLength -
                                x -
                                footprintLength;

                            y =
                                palletWidth -
                                y -
                                footprintWidth;

                            rotation += 180.0;
                        }


                        Matrix4x4 pose;

                        pose.setTranslation(
                            x,
                            y,
                            z
                        );

                        pose.setRotationZ(
                            rotation
                        );


                        Placement placement(
                            boxesPlaced + 1,
                            palletId,
                            pose
                        );


                        result.getPlacements()
                            .push_back(
                                placement
                            );


                        ++boxesPlaced;

                        ++boxesOnCurrentPallet;
                    }
                }
            }
        }


        // ====================================================
        // STATISTICS FOR CURRENT PALLET
        // ====================================================

        if (boxesOnCurrentPallet ==
            boxesPerPallet)
        {
            ++fullPallets;
        }
        else
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
        // Safety check
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

    cout << "ROWS + COLUMNS RESULT"
         << endl;

    cout << "----------------------------------------"
         << endl;

    cout << "Boxes requested: "
         << quantity
         << endl;

    cout << "Boxes placed   : "
         << boxesPlaced
         << endl;

    cout << "Full pallets   : "
         << fullPallets
         << endl;

    cout << "----------------------------------------"
         << endl;


    return result;
}