#include "WheelAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;


namespace
{
    constexpr double EPSILON = 1e-6;


    double normalizeAngle(double angle)
    {
        while (angle >= 360.0)
        {
            angle -= 360.0;
        }

        while (angle < 0.0)
        {
            angle += 360.0;
        }

        return angle;
    }
}


/*
 * ============================================================
 * PINWHEEL ALGORITHM
 * ============================================================
 *
 * The old Wheel implementation generated a rectangular
 * perimeter and then placed rotated side boxes using the
 * ORIGINAL box spacing.
 *
 * Example:
 *
 *     Box = 300 x 200
 *
 * After 90 degree rotation:
 *
 *     footprint = 200 x 300
 *
 * But the old code advanced Y by only 200 mm.
 *
 * Therefore:
 *
 *     Box 1 -> Y 200 to 500
 *     Box 2 -> Y 400 to 700
 *
 * They overlap between Y = 400 and Y = 500.
 *
 *
 * This implementation does not use that approach.
 *
 * Instead, it creates a real four-box pinwheel unit.
 *
 *
 *                 LONG
 *          +------------------+----+
 *          |                  |    |
 *          |       A          | B  |
 *          |                  |    |
 *          +------------------+    |
 *          | D |              |    |
 *          |   |  CENTRAL     |    |
 *          |   |    GAP       | C  |
 *          |   |              |    |
 *          +---+--------------+----+
 *              <---- LONG ---->
 *
 *
 * Unit dimensions:
 *
 *     (Long + Short) x (Long + Short)
 *
 *
 * Four boxes:
 *
 *     A = horizontal
 *     B = vertical
 *     C = horizontal
 *     D = vertical
 *
 *
 * Every placement is then geometrically validated.
 *
 * No overlap is allowed.
 * ============================================================
 */


WheelAlgorithm::Candidate
WheelAlgorithm::buildCandidate(
    const Pallet& pallet,
    const Box& box,
    bool reversePattern) const
{
    Candidate candidate;


    const double boxLength =
        box.getLength();

    const double boxWidth =
        box.getWidth();


    if (boxLength <= 0.0 ||
        boxWidth <= 0.0 ||
        pallet.getLength() <= 0.0 ||
        pallet.getWidth() <= 0.0)
    {
        return candidate;
    }


    /*
     * Work using long and short dimensions.
     *
     * This makes the geometry independent of whether
     * the user entered the larger dimension as length
     * or width.
     */
    const double longSide =
        max(boxLength, boxWidth);

    const double shortSide =
        min(boxLength, boxWidth);


    /*
     * A four-box pinwheel unit has this size:
     *
     *             Long + Short
     *       <----------------------->
     *
     *       +-----------------------+
     *       |                       |
     *       |       PINWHEEL        |
     *       |                       |
     *       +-----------------------+
     */
    const double unitSize =
        longSide + shortSide;


    if (unitSize <= EPSILON)
    {
        return candidate;
    }


    /*
     * Find how many complete pinwheel units
     * fit along the pallet.
     */
    const int unitsX =
        static_cast<int>(
            floor(
                (pallet.getLength() + EPSILON) /
                unitSize
            )
        );


    const int unitsY =
        static_cast<int>(
            floor(
                (pallet.getWidth() + EPSILON) /
                unitSize
            )
        );


    if (unitsX <= 0 ||
        unitsY <= 0)
    {
        return candidate;
    }


    candidate.unitsAlongLength =
        unitsX;

    candidate.unitsAlongWidth =
        unitsY;

    candidate.unitSize =
        unitSize;


    candidate.usedLength =
        unitsX *
        unitSize;

    candidate.usedWidth =
        unitsY *
        unitSize;


    /*
     * Center the complete pinwheel pattern
     * on the pallet.
     */
    const double offsetX =
        (pallet.getLength() -
         candidate.usedLength) /
        2.0;


    const double offsetY =
        (pallet.getWidth() -
         candidate.usedWidth) /
        2.0;


    /*
     * Determine which Z rotation represents
     * the long side pointing along X.
     */
    const double horizontalRotation =
        boxLength >= boxWidth
            ? 0.0
            : 90.0;


    const double verticalRotation =
        horizontalRotation +
        90.0;


    /*
     * ========================================================
     * BUILD PINWHEEL UNITS
     * ========================================================
     *
     * Each unit contains exactly four boxes.
     *
     * A:
     *
     *     x = 0
     *     y = 0
     *     size = Long x Short
     *
     * B:
     *
     *     x = Long
     *     y = 0
     *     size = Short x Long
     *
     * C:
     *
     *     x = Short
     *     y = Long
     *     size = Long x Short
     *
     * D:
     *
     *     x = 0
     *     y = Short
     *     size = Short x Long
     *
     *
     * The center gap is intentional.
     *
     * We NEVER try to squeeze a box into it.
     * ========================================================
     */

    for (int unitY = 0;
         unitY < unitsY;
         ++unitY)
    {
        for (int unitX = 0;
             unitX < unitsX;
             ++unitX)
        {
            const double originX =
                offsetX +
                unitX * unitSize;


            const double originY =
                offsetY +
                unitY * unitSize;


            vector<Footprint> unit;


            /*
             * ------------------------------------------------
             * BOX A
             * ------------------------------------------------
             */
            unit.push_back(
            {
                originX,
                originY,

                longSide,
                shortSide,

                horizontalRotation
            });


            /*
             * ------------------------------------------------
             * BOX B
             * ------------------------------------------------
             */
            unit.push_back(
            {
                originX + longSide,
                originY,

                shortSide,
                longSide,

                verticalRotation
            });


            /*
             * ------------------------------------------------
             * BOX C
             * ------------------------------------------------
             */
            unit.push_back(
            {
                originX + shortSide,
                originY + longSide,

                longSide,
                shortSide,

                horizontalRotation
            });


            /*
             * ------------------------------------------------
             * BOX D
             * ------------------------------------------------
             */
            unit.push_back(
            {
                originX,
                originY + shortSide,

                shortSide,
                longSide,

                verticalRotation
            });


            /*
             * Optional 180 degree variation for alternating
             * units.
             *
             * This does NOT change the footprint dimensions.
             * It only changes the handedness/orientation.
             */
            if (reversePattern &&
                ((unitX + unitY) % 2 == 1))
            {
                for (Footprint& footprint : unit)
                {
                    const double localX =
                        footprint.x -
                        originX;


                    const double localY =
                        footprint.y -
                        originY;


                    const double oldLength =
                        footprint.length;


                    const double oldWidth =
                        footprint.width;


                    footprint.x =
                        originX +
                        unitSize -
                        localX -
                        oldLength;


                    footprint.y =
                        originY +
                        unitSize -
                        localY -
                        oldWidth;


                    footprint.rotationZ =
                        normalizeAngle(
                            footprint.rotationZ +
                            180.0
                        );
                }
            }


            /*
             * Add the four boxes to the complete layer.
             */
            for (const Footprint& footprint : unit)
            {
                candidate.positions.push_back(
                    footprint
                );
            }
        }
    }


    /*
     * ========================================================
     * HARD GEOMETRIC VALIDATION
     * ========================================================
     *
     * Even though the pattern is mathematically generated,
     * we still validate EVERYTHING.
     *
     * This is the safety net that the old Wheel algorithm
     * did not have.
     */
    if (!isValidLayer(
            candidate.positions,
            pallet))
    {
        candidate.positions.clear();

        return candidate;
    }


    candidate.boxesPerLayer =
        static_cast<int>(
            candidate.positions.size()
        );


    /*
     * Calculate unused pallet area.
     */
    candidate.unusedArea =
        pallet.getLength() *
        pallet.getWidth()
        -
        candidate.boxesPerLayer *
        longSide *
        shortSide;


    if (candidate.unusedArea < 0.0 &&
        candidate.unusedArea > -EPSILON)
    {
        candidate.unusedArea = 0.0;
    }


    candidate.valid =
        candidate.boxesPerLayer > 0;


    return candidate;
}


/*
 * ============================================================
 * PALLET BOUNDARY CHECK
 * ============================================================
 */

bool WheelAlgorithm::fitsOnPallet(
    const Footprint& footprint,
    const Pallet& pallet) const
{
    if (footprint.x < -EPSILON)
    {
        return false;
    }


    if (footprint.y < -EPSILON)
    {
        return false;
    }


    if (footprint.x +
        footprint.length >
        pallet.getLength() +
        EPSILON)
    {
        return false;
    }


    if (footprint.y +
        footprint.width >
        pallet.getWidth() +
        EPSILON)
    {
        return false;
    }


    return true;
}


/*
 * ============================================================
 * BOX-BOX OVERLAP CHECK
 * ============================================================
 *
 * Positive-area intersection = overlap.
 *
 * Touching edges are allowed.
 *
 * Example:
 *
 *     Box A | Box B
 *
 * They share an edge but do not overlap.
 *
 * That is valid.
 * ============================================================
 */

bool WheelAlgorithm::overlaps(
    const Footprint& first,
    const Footprint& second) const
{
    const double firstRight =
        first.x +
        first.length;


    const double firstTop =
        first.y +
        first.width;


    const double secondRight =
        second.x +
        second.length;


    const double secondTop =
        second.y +
        second.width;


    const double overlapX =
        min(
            firstRight,
            secondRight
        )
        -
        max(
            first.x,
            second.x
        );


    const double overlapY =
        min(
            firstTop,
            secondTop
        )
        -
        max(
            first.y,
            second.y
        );


    return
        overlapX > EPSILON &&
        overlapY > EPSILON;
}


/*
 * ============================================================
 * COMPLETE LAYER VALIDATION
 * ============================================================
 */

bool WheelAlgorithm::isValidLayer(
    const vector<Footprint>& positions,
    const Pallet& pallet) const
{
    /*
     * First check every box against the pallet.
     */
    for (const Footprint& footprint :
         positions)
    {
        if (!fitsOnPallet(
                footprint,
                pallet))
        {
            return false;
        }
    }


    /*
     * Then check every box against every
     * other box.
     */
    for (size_t i = 0;
         i < positions.size();
         ++i)
    {
        for (size_t j = i + 1;
             j < positions.size();
             ++j)
        {
            if (overlaps(
                    positions[i],
                    positions[j]))
            {
                return false;
            }
        }
    }


    return !positions.empty();
}


/*
 * ============================================================
 * CREATE PLACEMENT
 * ============================================================
 */

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


/*
 * ============================================================
 * MAIN ALGORITHM
 * ============================================================
 */

PalletizationResult
WheelAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;


    /*
     * --------------------------------------------------------
     * BASIC VALIDATION
     * --------------------------------------------------------
     */

    if (quantity <= 0 ||
        box.getLength() <= 0.0 ||
        box.getWidth() <= 0.0 ||
        box.getHeight() <= 0.0 ||
        pallet.getLength() <= 0.0 ||
        pallet.getWidth() <= 0.0 ||
        pallet.getHeight() <= 0.0)
    {
        return result;
    }


    /*
     * --------------------------------------------------------
     * BUILD TWO PINWHEEL VARIANTS
     * --------------------------------------------------------
     */

    Candidate candidates[2];


    candidates[0] =
        buildCandidate(
            pallet,
            box,
            false
        );


    candidates[1] =
        buildCandidate(
            pallet,
            box,
            true
        );


    Candidate bestCandidate;

    bool foundCandidate =
        false;


    /*
     * Select the valid candidate with the
     * greatest number of boxes per layer.
     */
    for (const Candidate& candidate :
         candidates)
    {
        if (!candidate.valid)
        {
            continue;
        }


        if (!foundCandidate ||
            candidate.boxesPerLayer >
                bestCandidate.boxesPerLayer ||
            (
                candidate.boxesPerLayer ==
                    bestCandidate.boxesPerLayer &&
                candidate.unusedArea <
                    bestCandidate.unusedArea -
                    EPSILON
            ))
        {
            bestCandidate =
                candidate;

            foundCandidate =
                true;
        }
    }


    /*
     * No valid pinwheel can fit.
     */
    if (!foundCandidate)
    {
        cout
            << "PINWHEEL: No valid pinwheel layer "
            << "fits on the pallet."
            << endl;

        return result;
    }


    /*
     * --------------------------------------------------------
     * NUMBER OF VERTICAL LAYERS
     * --------------------------------------------------------
     */

    const int layers =
        static_cast<int>(
            floor(
                (pallet.getHeight() + EPSILON) /
                box.getHeight()
            )
        );


    if (layers <= 0)
    {
        return result;
    }


    /*
     * Total capacity of one pallet.
     */
    const int boxesPerPallet =
        bestCandidate.boxesPerLayer *
        layers;


    /*
     * --------------------------------------------------------
     * VOLUME INFORMATION
     * --------------------------------------------------------
     */

    const double boxVolume =
        box.getLength() *
        box.getWidth() *
        box.getHeight();


    const double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();


    /*
     * --------------------------------------------------------
     * DEBUG INFORMATION
     * --------------------------------------------------------
     */

    cout
        << "\n========================================"
        << endl;


    cout
        << "PINWHEEL ALGORITHM"
        << endl;


    cout
        << "========================================"
        << endl;


    cout
        << "Boxes per pinwheel layer: "
        << bestCandidate.boxesPerLayer
        << endl;


    cout
        << "Pinwheel units along L: "
        << bestCandidate.unitsAlongLength
        << endl;


    cout
        << "Pinwheel units along W: "
        << bestCandidate.unitsAlongWidth
        << endl;


    cout
        << "Pinwheel unit size: "
        << bestCandidate.unitSize
        << " x "
        << bestCandidate.unitSize
        << endl;


    cout
        << "Layers per pallet: "
        << layers
        << endl;


    cout
        << "Boxes per pallet: "
        << boxesPerPallet
        << endl;


    cout
        << "========================================"
        << endl;


    /*
     * --------------------------------------------------------
     * PLACE BOXES
     * --------------------------------------------------------
     */

    int boxesPlaced = 0;

    int fullPallets = 0;

    int palletId = 1;


    while (boxesPlaced < quantity)
    {
        const int boxesBeforePallet =
            boxesPlaced;


        /*
         * Generate every layer.
         */
        for (int layer = 0;
             layer < layers &&
             boxesPlaced < quantity;
             ++layer)
        {
            const double z =
                layer *
                box.getHeight();


            /*
             * Alternate layers by rotating the
             * complete pinwheel 180 degrees.
             *
             * This gives the stack an interlocking
             * layer-to-layer effect.
             */
            const bool flippedLayer =
                (layer % 2 == 1);


            for (const Footprint& footprint :
                 bestCandidate.positions)
            {
                if (boxesPlaced >= quantity)
                {
                    break;
                }


                double x =
                    footprint.x;


                double y =
                    footprint.y;


                double rotation =
                    footprint.rotationZ;


                /*
                 * Flip the complete layer around
                 * the pallet centre.
                 */
                if (flippedLayer)
                {
                    x =
                        pallet.getLength() -
                        footprint.x -
                        footprint.length;


                    y =
                        pallet.getWidth() -
                        footprint.y -
                        footprint.width;


                    rotation =
                        normalizeAngle(
                            rotation +
                            180.0
                        );
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
            }
        }


        /*
         * ----------------------------------------------------
         * PALLET STATISTICS
         * ----------------------------------------------------
         */

        const int boxesOnCurrentPallet =
            boxesPlaced -
            boxesBeforePallet;


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


        /*
         * Prevent an infinite loop if something
         * unexpected happens.
         */
        if (boxesOnCurrentPallet == 0)
        {
            break;
        }


        ++palletId;
    }


    /*
     * --------------------------------------------------------
     * FINAL STATISTICS
     * --------------------------------------------------------
     */

    result.getStatistics()
        .setTotalBoxes(
            boxesPlaced
        );


    result.getStatistics()
        .setFullPallets(
            fullPallets
        );


    cout
        << "Boxes requested: "
        << quantity
        << endl;


    cout
        << "Boxes placed: "
        << boxesPlaced
        << endl;


    cout
        << "Full pallets: "
        << fullPallets
        << endl;


    return result;
}