#include "WheelInnerFillAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

namespace
{
    constexpr double EPSILON = 1e-6;

    double normalizedAngle(double angle)
    {
        while (angle >= 360.0)
            angle -= 360.0;

        while (angle < 0.0)
            angle += 360.0;

        return angle;
    }
}

WheelInnerFillAlgorithm::Candidate
WheelInnerFillAlgorithm::buildCandidate(
    const Pallet& pallet,
    const Box& box,
    int topBottomBoxes,
    int sideBoxes,
    bool innerRotated) const
{
    Candidate candidate;

    if (topBottomBoxes < 2 || sideBoxes < 1)
        return candidate;

    const double boxLength = box.getLength();
    const double boxWidth = box.getWidth();

    if (boxLength <= EPSILON || boxWidth <= EPSILON)
        return candidate;

    const double longSide = max(boxLength, boxWidth);
    const double shortSide = min(boxLength, boxWidth);

    /*
     * We use the long side horizontally for the top/bottom
     * perimeter and the short side as the frame thickness.
     *
     * This gives the characteristic four-direction pinwheel
     * perimeter:
     *
     *   TOP       ->  long x short
     *   LEFT/RIGHT->  short x long
     *   BOTTOM    ->  long x short
     *
     * The side boxes begin after the top row and end before
     * the bottom row, so they cannot overlap the corners.
     */
    const double outerLength =
        topBottomBoxes * longSide;

    const double outerWidth =
        2.0 * shortSide +
        sideBoxes * longSide;

    if (outerLength > pallet.getLength() + EPSILON ||
        outerWidth > pallet.getWidth() + EPSILON)
    {
        return candidate;
    }

    const double innerStartX = shortSide;
    const double innerStartY = shortSide;

    const double innerLength =
        outerLength - 2.0 * shortSide;

    const double innerWidth =
        outerWidth - 2.0 * shortSide;

    if (innerLength <= EPSILON || innerWidth <= EPSILON)
        return candidate;

    const double innerLengthDimension =
        innerRotated ? shortSide : longSide;

    const double innerWidthDimension =
        innerRotated ? longSide : shortSide;

    const double innerRotation =
        innerRotated
            ? (boxLength >= boxWidth ? 90.0 : 0.0)
            : (boxLength >= boxWidth ? 0.0 : 90.0);

    const int innerAlongLength =
        static_cast<int>(floor(
            (innerLength + EPSILON) /
            innerLengthDimension));

    const int innerAlongWidth =
        static_cast<int>(floor(
            (innerWidth + EPSILON) /
            innerWidthDimension));

    /*
     * Center the complete hybrid pattern on the pallet.
     */
    const double offsetX =
        max(0.0,
            (pallet.getLength() - outerLength) / 2.0);

    const double offsetY =
        max(0.0,
            (pallet.getWidth() - outerWidth) / 2.0);

    const double horizontalRotation =
        boxLength >= boxWidth ? 0.0 : 90.0;

    const double verticalRotation =
        normalizedAngle(horizontalRotation + 90.0);

    vector<Footprint> positions;
    positions.reserve(
        2 * topBottomBoxes +
        2 * sideBoxes +
        max(0, innerAlongLength * innerAlongWidth));

    /*
     * ============================================================
     * OUTER PINWHEEL PERIMETER
     * ============================================================
     *
     * TOP
     */
    for (int i = 0; i < topBottomBoxes; ++i)
    {
        positions.push_back(
        {
            offsetX + i * longSide,
            offsetY,
            longSide,
            shortSide,
            horizontalRotation
        });
    }

    /*
     * BOTTOM
     */
    for (int i = 0; i < topBottomBoxes; ++i)
    {
        positions.push_back(
        {
            offsetX + i * longSide,
            offsetY + outerWidth - shortSide,
            longSide,
            shortSide,
            horizontalRotation
        });
    }

    /*
     * LEFT SIDE
     *
     * Starts below the top row and therefore touches it instead
     * of penetrating it.
     */
    for (int j = 0; j < sideBoxes; ++j)
    {
        positions.push_back(
        {
            offsetX,
            offsetY + shortSide + j * longSide,
            shortSide,
            longSide,
            verticalRotation
        });
    }

    /*
     * RIGHT SIDE
     */
    for (int j = 0; j < sideBoxes; ++j)
    {
        positions.push_back(
        {
            offsetX + outerLength - shortSide,
            offsetY + shortSide + j * longSide,
            shortSide,
            longSide,
            verticalRotation
        });
    }

    /*
     * ============================================================
     * INNER FILL
     * ============================================================
     *
     * The fill starts exactly at the inside edge of the frame.
     * Therefore there is no artificial empty column caused by
     * incorrect side-box spacing.
     */
    for (int row = 0; row < innerAlongWidth; ++row)
    {
        for (int col = 0; col < innerAlongLength; ++col)
        {
            positions.push_back(
            {
                offsetX +
                    innerStartX +
                    col * innerLengthDimension,

                offsetY +
                    innerStartY +
                    row * innerWidthDimension,

                innerLengthDimension,
                innerWidthDimension,
                innerRotation
            });
        }
    }

    if (!isValidLayer(positions, pallet))
        return candidate;

    candidate.valid = true;
    candidate.positions = positions;
    candidate.perimeterBoxes =
        2 * topBottomBoxes + 2 * sideBoxes;
    candidate.innerBoxes =
        innerAlongLength * innerAlongWidth;
    candidate.boxesPerLayer =
        static_cast<int>(positions.size());
    candidate.topBottomBoxes = topBottomBoxes;
    candidate.sideBoxes = sideBoxes;
    candidate.innerAlongLength = innerAlongLength;
    candidate.innerAlongWidth = innerAlongWidth;
    candidate.usedLength = outerLength;
    candidate.usedWidth = outerWidth;

    candidate.unusedArea = max(
        0.0,
        pallet.getLength() * pallet.getWidth() -
        boxLength * boxWidth *
            static_cast<double>(candidate.boxesPerLayer));

    return candidate;
}

WheelInnerFillAlgorithm::Candidate
WheelInnerFillAlgorithm::findBestCandidate(
    const Pallet& pallet,
    const Box& box) const
{
    Candidate best;

    const double longSide =
        max(box.getLength(), box.getWidth());

    const double shortSide =
        min(box.getLength(), box.getWidth());

    if (longSide <= EPSILON || shortSide <= EPSILON)
        return best;

    const int maxTopBottom =
        static_cast<int>(floor(
            (pallet.getLength() + EPSILON) /
            longSide));

    const int maxSide =
        static_cast<int>(floor(
            (pallet.getWidth() -
             2.0 * shortSide + EPSILON) /
            longSide));

    if (maxTopBottom < 2 || maxSide < 1)
        return best;

    /*
     * Search all feasible frame sizes and both inner orientations.
     *
     * Primary objective:
     *   more boxes per layer.
     *
     * Secondary objectives:
     *   more compact use of the pallet and fewer unused strips.
     */
    for (int topBottom = 2;
         topBottom <= maxTopBottom;
         ++topBottom)
    {
        for (int side = 1;
             side <= maxSide;
             ++side)
        {
            for (bool innerRotated : {false, true})
            {
                Candidate candidate =
                    buildCandidate(
                        pallet,
                        box,
                        topBottom,
                        side,
                        innerRotated);

                if (!candidate.valid)
                    continue;

                const double candidateArea =
                    candidate.usedLength *
                    candidate.usedWidth;

                const double bestArea =
                    best.usedLength *
                    best.usedWidth;

                if (!best.valid ||
                    candidate.boxesPerLayer >
                        best.boxesPerLayer ||
                    (candidate.boxesPerLayer ==
                         best.boxesPerLayer &&
                     candidateArea > bestArea) ||
                    (candidate.boxesPerLayer ==
                         best.boxesPerLayer &&
                     fabs(candidateArea - bestArea) <= EPSILON &&
                     candidate.unusedArea <
                         best.unusedArea))
                {
                    best = candidate;
                }
            }
        }
    }

    return best;
}

bool WheelInnerFillAlgorithm::fitsOnPallet(
    const Footprint& footprint,
    const Pallet& pallet) const
{
    return footprint.x >= -EPSILON &&
           footprint.y >= -EPSILON &&
           footprint.x + footprint.length <=
               pallet.getLength() + EPSILON &&
           footprint.y + footprint.width <=
               pallet.getWidth() + EPSILON;
}

bool WheelInnerFillAlgorithm::overlaps(
    const Footprint& first,
    const Footprint& second) const
{
    const double overlapX =
        min(first.x + first.length,
            second.x + second.length) -
        max(first.x, second.x);

    const double overlapY =
        min(first.y + first.width,
            second.y + second.width) -
        max(first.y, second.y);

    return overlapX > EPSILON &&
           overlapY > EPSILON;
}

bool WheelInnerFillAlgorithm::isValidLayer(
    const vector<Footprint>& positions,
    const Pallet& pallet) const
{
    if (positions.empty())
        return false;

    for (const Footprint& footprint : positions)
    {
        if (!fitsOnPallet(footprint, pallet))
            return false;

        if (footprint.length <= EPSILON ||
            footprint.width <= EPSILON)
        {
            return false;
        }
    }

    for (size_t i = 0; i < positions.size(); ++i)
    {
        for (size_t j = i + 1;
             j < positions.size();
             ++j)
        {
            if (overlaps(positions[i], positions[j]))
                return false;
        }
    }

    return true;
}

void WheelInnerFillAlgorithm::addPlacement(
    PalletizationResult& result,
    int boxId,
    int palletId,
    double x,
    double y,
    double z,
    double rotationZ) const
{
    Matrix4x4 pose;
    pose.setTranslation(x, y, z);
    pose.setRotationZ(normalizedAngle(rotationZ));

    result.getPlacements().push_back(
        Placement(boxId, palletId, pose));
}

PalletizationResult
WheelInnerFillAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;

    if (quantity <= 0 ||
        box.getLength() <= EPSILON ||
        box.getWidth() <= EPSILON ||
        box.getHeight() <= EPSILON ||
        pallet.getLength() <= EPSILON ||
        pallet.getWidth() <= EPSILON ||
        pallet.getHeight() <= EPSILON)
    {
        return result;
    }

    Candidate candidate =
        findBestCandidate(pallet, box);

    if (!candidate.valid || candidate.boxesPerLayer <= 0)
        return result;

    const int layersPerPallet =
        static_cast<int>(floor(
            (pallet.getHeight() + EPSILON) /
            box.getHeight()));

    if (layersPerPallet <= 0)
        return result;

    const int boxesPerPallet =
        candidate.boxesPerLayer * layersPerPallet;

    int boxesPlaced = 0;
    int fullPallets = 0;
    int palletId = 1;

    const double boxVolume =
        box.getLength() *
        box.getWidth() *
        box.getHeight();

    const double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();

    while (boxesPlaced < quantity)
    {
        const int palletStart = boxesPlaced;

        for (int layer = 0;
             layer < layersPerPallet &&
             boxesPlaced < quantity;
             ++layer)
        {
            const double z =
                layer * box.getHeight();

            /*
             * Alternate the handedness of the complete layer.
             * 180 degrees leaves the footprint unchanged but
             * changes the visual orientation of the pattern.
             */
            const double layerRotation =
                (layer % 2 == 1) ? 180.0 : 0.0;

            for (const Footprint& footprint : candidate.positions)
            {
                if (boxesPlaced >= quantity)
                    break;

                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    footprint.x,
                    footprint.y,
                    z,
                    footprint.rotationZ + layerRotation);

                ++boxesPlaced;
            }
        }

        const int boxesOnPallet =
            boxesPlaced - palletStart;

        if (boxesOnPallet == boxesPerPallet)
        {
            ++fullPallets;
        }
        else if (boxesOnPallet > 0)
        {
            result.getStatistics().setLastPalletStatistics(
                PalletStatistics(
                    palletId,
                    boxesOnPallet * boxVolume,
                    palletVolume));
        }

        ++palletId;

        if (boxesOnPallet <= 0)
            break;
    }

    result.getStatistics().setTotalBoxes(boxesPlaced);
    result.getStatistics().setFullPallets(fullPallets);

    cout << "\nHYBRID PINWHEEL ALGORITHM" << endl;
    cout << "Boxes per layer: "
         << candidate.boxesPerLayer << endl;
    cout << "Perimeter boxes: "
         << candidate.perimeterBoxes << endl;
    cout << "Inner fill boxes: "
         << candidate.innerBoxes << endl;
    cout << "Top/Bottom boxes per row: "
         << candidate.topBottomBoxes << endl;
    cout << "Side boxes per side: "
         << candidate.sideBoxes << endl;
    cout << "Inner fill grid: "
         << candidate.innerAlongLength
         << " x "
         << candidate.innerAlongWidth << endl;
    cout << "Layers per pallet: "
         << layersPerPallet << endl;
    cout << "Pallet capacity: "
         << boxesPerPallet << endl;
    cout << "Geometric validation: PASSED" << endl;

    return result;
}
