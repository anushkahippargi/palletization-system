
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "BricksAlgorithm.h"

using namespace std;

namespace
{
    constexpr double EPSILON = 1e-9;
    constexpr double MIN_SUPPORT_RATIO = 0.50;

    struct BrickPosition
    {
        double x = 0.0;
        double y = 0.0;
        double footprintLength = 0.0;
        double footprintWidth = 0.0;
        double rotationZ = 0.0;
        int rowIndex = 0;
    };

    struct RowBand
    {
        double width = 0.0;
        double boxLength = 0.0;
        double startX = 0.0;
        double rotationZ = 0.0;
        int boxes = 0;
    };

    struct LayerLayout
    {
        vector<BrickPosition> positions;
        vector<RowBand> rows;
        int count = 0;
        int orientationChanges = 0;
        double usedWidth = 0.0;
        bool valid = false;
    };

    bool isBetterLayout(const LayerLayout& candidate,
                        const LayerLayout& best)
    {
        if (!candidate.valid)
            return false;

        if (!best.valid)
            return true;

        if (candidate.count != best.count)
            return candidate.count > best.count;

        if (candidate.orientationChanges != best.orientationChanges)
            return candidate.orientationChanges > best.orientationChanges;

        return candidate.usedWidth < best.usedWidth - EPSILON;
    }

    void searchRows(double palletLength,
                    double palletWidth,
                    double boxLength,
                    double boxWidth,
                    double currentY,
                    int phase,
                    vector<BrickPosition>& positions,
                    vector<RowBand>& rows,
                    int currentCount,
                    int orientationChanges,
                    LayerLayout& best)
    {
        LayerLayout candidate;
        candidate.positions = positions;
        candidate.rows = rows;
        candidate.count = currentCount;
        candidate.orientationChanges = orientationChanges;
        candidate.usedWidth = currentY;
        candidate.valid = currentCount > 0;

        if (isBetterLayout(candidate, best))
            best = candidate;

        if (currentY >= palletWidth - EPSILON)
            return;

        for (int orientation = 0; orientation < 2; ++orientation)
        {
            const double rowLength =
                orientation == 0 ? boxLength : boxWidth;

            const double rowWidth =
                orientation == 0 ? boxWidth : boxLength;

            const double rotationZ =
                orientation == 0 ? 0.0 : 90.0;

            if (rowLength <= 0.0 || rowWidth <= 0.0 ||
                currentY + rowWidth > palletWidth + EPSILON)
            {
                continue;
            }

            const int rowIndex = static_cast<int>(rows.size());

            // Alternate the half-box offset to create a brick-like pattern.
            const bool shifted = ((rowIndex + phase) % 2 == 1);
            const double startX = shifted ? rowLength / 2.0 : 0.0;

            const int boxesInRow = static_cast<int>(
                floor((palletLength - startX + EPSILON) / rowLength));

            if (boxesInRow <= 0)
                continue;

            int change = 0;

            if (!rows.empty())
            {
                const bool previousRotated =
                    rows.back().rotationZ != 0.0;

                const bool currentRotated = rotationZ != 0.0;

                change = previousRotated != currentRotated ? 1 : 0;
            }

            const size_t oldPositionSize = positions.size();
            const size_t oldRowSize = rows.size();

            rows.push_back({
                rowWidth,
                rowLength,
                startX,
                rotationZ,
                boxesInRow
            });

            for (int i = 0; i < boxesInRow; ++i)
            {
                positions.push_back({
                    startX + i * rowLength,
                    currentY,
                    rowLength,
                    rowWidth,
                    rotationZ,
                    rowIndex
                });
            }

            searchRows(palletLength,
                       palletWidth,
                       boxLength,
                       boxWidth,
                       currentY + rowWidth,
                       phase,
                       positions,
                       rows,
                       currentCount + boxesInRow,
                       orientationChanges + change,
                       best);

            positions.resize(oldPositionSize);
            rows.resize(oldRowSize);
        }
    }

    LayerLayout createLayer(double palletLength,
                            double palletWidth,
                            double boxLength,
                            double boxWidth,
                            int phase)
    {
        LayerLayout best;
        vector<BrickPosition> positions;
        vector<RowBand> rows;

        searchRows(palletLength,
                   palletWidth,
                   boxLength,
                   boxWidth,
                   0.0,
                   phase,
                   positions,
                   rows,
                   0,
                   0,
                   best);

        return best;
    }

    bool validateLayer(const LayerLayout& layout,
                       double palletLength,
                       double palletWidth)
    {
        if (!layout.valid || layout.positions.empty())
            return false;

        for (const BrickPosition& box : layout.positions)
        {
            if (box.x < -EPSILON || box.y < -EPSILON ||
                box.x + box.footprintLength > palletLength + EPSILON ||
                box.y + box.footprintWidth > palletWidth + EPSILON)
            {
                return false;
            }
        }

        // Check that no two boxes overlap within this layer.
        for (size_t i = 0; i < layout.positions.size(); ++i)
        {
            const BrickPosition& a = layout.positions[i];

            for (size_t j = i + 1; j < layout.positions.size(); ++j)
            {
                const BrickPosition& b = layout.positions[j];

                const double overlapX =
                    min(a.x + a.footprintLength,
                        b.x + b.footprintLength) -
                    max(a.x, b.x);

                const double overlapY =
                    min(a.y + a.footprintWidth,
                        b.y + b.footprintWidth) -
                    max(a.y, b.y);

                if (overlapX > EPSILON && overlapY > EPSILON)
                    return false;
            }
        }

        return true;
    }

    bool validateSupport(const LayerLayout& lower,
                         const LayerLayout& upper)
    {
        if (lower.positions.empty() || upper.positions.empty())
            return false;

        // The lower layer has already passed overlap validation, so
        // supported areas can be added without double-counting.
        for (const BrickPosition& upperBox : upper.positions)
        {
            const double upperArea =
                upperBox.footprintLength * upperBox.footprintWidth;

            if (upperArea <= EPSILON)
                return false;

            double supportedArea = 0.0;

            for (const BrickPosition& lowerBox : lower.positions)
            {
                const double overlapX =
                    min(upperBox.x + upperBox.footprintLength,
                        lowerBox.x + lowerBox.footprintLength) -
                    max(upperBox.x, lowerBox.x);

                const double overlapY =
                    min(upperBox.y + upperBox.footprintWidth,
                        lowerBox.y + lowerBox.footprintWidth) -
                    max(upperBox.y, lowerBox.y);

                if (overlapX > EPSILON && overlapY > EPSILON)
                    supportedArea += overlapX * overlapY;
            }

            const double supportRatio = supportedArea / upperArea;

            if (supportRatio + EPSILON < MIN_SUPPORT_RATIO)
                return false;
        }

        return true;
    }
}

int BricksAlgorithm::calculateBoxesAlong(double palletDimension,
                                         double boxDimension) const
{
    if (boxDimension <= 0.0)
        return 0;

    return static_cast<int>(
        floor((palletDimension + EPSILON) / boxDimension));
}

BricksAlgorithm::Candidate BricksAlgorithm::evaluateCandidate(
    const Pallet& pallet,
    const Orientation& orientation,
    int boxesAlongLength,
    int boxesAlongWidth) const
{
    Candidate candidate;

    if (boxesAlongLength <= 0 || boxesAlongWidth <= 0)
        return candidate;

    const double usedLength = boxesAlongLength * orientation.length;
    const double usedWidth = boxesAlongWidth * orientation.width;

    if (usedLength > pallet.getLength() + EPSILON ||
        usedWidth > pallet.getWidth() + EPSILON)
    {
        return candidate;
    }

    candidate.valid = true;
    candidate.boxesAlongLength = boxesAlongLength;
    candidate.boxesAlongWidth = boxesAlongWidth;
    candidate.boxesPerLayer = boxesAlongLength * boxesAlongWidth;
    candidate.usedLength = usedLength;
    candidate.usedWidth = usedWidth;

    candidate.unusedArea =
        pallet.getLength() * pallet.getWidth() -
        usedLength * usedWidth;

    candidate.orientation = orientation;

    return candidate;
}

void BricksAlgorithm::addPlacement(PalletizationResult& result,
                                   int boxId,
                                   int palletId,
                                   double x,
                                   double y,
                                   double z,
                                   double rotationZ) const
{
    Matrix4x4 pose;

    pose.setTranslation(x, y, z);
    pose.setRotationZ(rotationZ);

    Placement placement(boxId, palletId, pose);
    result.getPlacements().push_back(placement);
}

PalletizationResult BricksAlgorithm::generatePattern(const Pallet& pallet,
                                                      const Box& box,
                                                      int quantity)
{
    PalletizationResult result;

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

    // Generate two staggered layer patterns.
    const LayerLayout layoutA = createLayer(
        pallet.getLength(),
        pallet.getWidth(),
        box.getLength(),
        box.getWidth(),
        0);

    const LayerLayout layoutB = createLayer(
        pallet.getLength(),
        pallet.getWidth(),
        box.getLength(),
        box.getWidth(),
        1);

    if (!layoutA.valid || layoutA.count <= 0)
        return result;

    const bool layoutAValid = validateLayer(
        layoutA, pallet.getLength(), pallet.getWidth());

    const bool layoutBValid = validateLayer(
        layoutB, pallet.getLength(), pallet.getWidth());

    // Alternating layers are allowed only if both transitions satisfy
    // the provisional support threshold.
    const bool canAlternate =
        layoutAValid &&
        layoutBValid &&
        validateSupport(layoutA, layoutB) &&
        validateSupport(layoutB, layoutA);

    const bool hasLayoutB = canAlternate;

    const int layersPerPallet = calculateBoxesAlong(
        pallet.getHeight(),
        box.getHeight());

    if (layersPerPallet <= 0)
        return result;

    // Calculate capacity using the actual layout selected for each layer.
    int palletCapacity = 0;

    for (int zIndex = 0; zIndex < layersPerPallet; ++zIndex)
    {
        if (zIndex % 2 == 1 && hasLayoutB)
            palletCapacity += layoutB.count;
        else
            palletCapacity += layoutA.count;
    }

    if (palletCapacity <= 0)
        return result;

    const double boxVolume =
        box.getLength() * box.getWidth() * box.getHeight();

    const double palletVolume =
        pallet.getLength() * pallet.getWidth() * pallet.getHeight();

    cout << "\nBRICKS ALGORITHM - STAGGERED LAYERS" << endl;
    cout << "Layer A boxes: " << layoutA.count << endl;
    cout << "Layer A rows: " << layoutA.rows.size() << endl;

    if (hasLayoutB)
    {
        cout << "Layer B boxes: " << layoutB.count << endl;
        cout << "Layer B rows: " << layoutB.rows.size() << endl;
        cout << "Alternating layers accepted: support and overlap checks passed."
             << endl;
    }
    else
    {
        cout << "Alternating layers rejected: geometry or support check failed."
             << endl;
        cout << "Using Layer A for every layer." << endl;
    }

    cout << "Minimum support threshold: "
         << MIN_SUPPORT_RATIO * 100.0 << "%" << endl;
    cout << "Layers per pallet: " << layersPerPallet << endl;
    cout << "Capacity per pallet: " << palletCapacity << endl;

    int boxesPlaced = 0;
    int fullPallets = 0;
    int palletId = 1;

    while (boxesPlaced < quantity)
    {
        const int beforePallet = boxesPlaced;

        for (int zIndex = 0;
             zIndex < layersPerPallet && boxesPlaced < quantity;
             ++zIndex)
        {
            const double z = zIndex * box.getHeight();

            const LayerLayout& currentLayout =
                (zIndex % 2 == 1 && hasLayoutB) ? layoutB : layoutA;

            for (const BrickPosition& position : currentLayout.positions)
            {
                if (boxesPlaced >= quantity)
                    break;

                addPlacement(result,
                             boxesPlaced + 1,
                             palletId,
                             position.x,
                             position.y,
                             z,
                             position.rotationZ);

                ++boxesPlaced;
            }
        }

        const int boxesOnCurrentPallet = boxesPlaced - beforePallet;

        if (boxesOnCurrentPallet == palletCapacity)
        {
            ++fullPallets;
        }
        else if (boxesOnCurrentPallet > 0)
        {
            PalletStatistics lastPallet(
                palletId,
                boxesOnCurrentPallet * boxVolume,
                palletVolume);

            result.getStatistics().setLastPalletStatistics(lastPallet);
        }

        ++palletId;

        if (boxesOnCurrentPallet == 0)
            break;
    }

    Statistics& statistics = result.getStatistics();

    statistics.setTotalBoxes(boxesPlaced);
    statistics.setFullPallets(fullPallets);

    return result;
}