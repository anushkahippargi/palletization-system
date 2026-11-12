
#include "PinwheelAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    constexpr double EPSILON = 1e-6;

    double normalizeAngle(double angle)
    {
        while (angle >= 360.0) angle -= 360.0;
        while (angle < 0.0) angle += 360.0;
        return angle;
    }
}

bool PinwheelAlgorithm::fitsOnPallet(
    const Footprint& f,
    const Pallet& pallet) const
{
    return f.x >= -EPSILON &&
           f.y >= -EPSILON &&
           f.length > EPSILON &&
           f.width > EPSILON &&
           f.x + f.length <= pallet.getLength() + EPSILON &&
           f.y + f.width <= pallet.getWidth() + EPSILON;
}

bool PinwheelAlgorithm::overlaps(
    const Footprint& a,
    const Footprint& b) const
{
    return std::min(a.x + a.length, b.x + b.length) -
               std::max(a.x, b.x) > EPSILON &&
           std::min(a.y + a.width, b.y + b.width) -
               std::max(a.y, b.y) > EPSILON;
}

bool PinwheelAlgorithm::isValidLayer(
    const std::vector<Footprint>& positions,
    const Pallet& pallet) const
{
    for (size_t i = 0; i < positions.size(); ++i)
    {
        if (!fitsOnPallet(positions[i], pallet))
            return false;

        for (size_t j = i + 1; j < positions.size(); ++j)
        {
            if (overlaps(positions[i], positions[j]))
                return false;
        }
    }

    return !positions.empty();
}

void PinwheelAlgorithm::addPlacement(
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
    pose.setRotationZ(normalizeAngle(rotationZ));

    result.getPlacements().push_back(
        Placement(boxId, palletId, pose));
}

PalletizationResult PinwheelAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;

    const double L = box.getLength();
    const double W = box.getWidth();
    const double H = box.getHeight();

    if (quantity <= 0 || L <= EPSILON || W <= EPSILON ||
        H <= EPSILON || pallet.getLength() <= EPSILON ||
        pallet.getWidth() <= EPSILON || pallet.getHeight() <= EPSILON)
        return result;

    const double P = pallet.getLength();
    const double Q = pallet.getWidth();

    std::vector<Footprint> best;
    int bestCount = 0;

    // Search both possible orientations for the pinwheel arms.
    for (int orientation = 0; orientation < 2; ++orientation)
    {
        const double a = orientation == 0 ? L : W;
        const double b = orientation == 0 ? W : L;
        const double horizontalRotation = orientation == 0 ? 0.0 : 90.0;
        const double verticalRotation = normalizeAngle(horizontalRotation + 90.0);

        const int maxAcross = static_cast<int>(P / a);
        const int maxSide = static_cast<int>((Q - 2.0 * b) / a);

        for (int across = 2; across <= maxAcross; ++across)
        {
            for (int side = 1; side <= maxSide; ++side)
            {
                const double frameLength = across * a;
                const double frameWidth = 2.0 * b + side * a;

                if (frameLength > P + EPSILON ||
                    frameWidth > Q + EPSILON)
                    continue;

                const double ox = (P - frameLength) / 2.0;
                const double oy = (Q - frameWidth) / 2.0;

                std::vector<Footprint> candidate;

                // Top and bottom arms.
                for (int i = 0; i < across; ++i)
                {
                    candidate.push_back({
                        ox + i * a, oy, a, b, horizontalRotation
                    });

                    candidate.push_back({
                        ox + i * a, oy + frameWidth - b,
                        a, b, horizontalRotation
                    });
                }

                // Left and right arms.
                for (int j = 0; j < side; ++j)
                {
                    candidate.push_back({
                        ox, oy + b + j * a, b, a, verticalRotation
                    });

                    candidate.push_back({
                        ox + frameLength - b, oy + b + j * a,
                        b, a, verticalRotation
                    });
                }

                if (!isValidLayer(candidate, pallet))
                    continue;

                if (static_cast<int>(candidate.size()) > bestCount)
                {
                    bestCount = static_cast<int>(candidate.size());
                    best = candidate;
                }
            }
        }
    }

    if (best.empty())
        return result;

    const int layersPerPallet =
        static_cast<int>(pallet.getHeight() / H);

    if (layersPerPallet <= 0)
        return result;

    const int boxesPerPallet = bestCount * layersPerPallet;
    const double boxVolume = L * W * H;
    const double palletVolume =
        pallet.getLength() * pallet.getWidth() * pallet.getHeight();

    int placed = 0;
    int fullPallets = 0;
    int palletId = 1;

    while (placed < quantity)
    {
        const int start = placed;

        for (int layer = 0;
             layer < layersPerPallet && placed < quantity;
             ++layer)
        {
            const double z = layer * H;
            const double flip = layer % 2 == 0 ? 0.0 : 180.0;

            for (const Footprint& f : best)
            {
                if (placed >= quantity)
                    break;

                addPlacement(
                    result, placed + 1, palletId,
                    f.x, f.y, z, f.rotationZ + flip);

                ++placed;
            }
        }

        const int onPallet = placed - start;

        if (onPallet == boxesPerPallet)
            ++fullPallets;
        else if (onPallet > 0)
        {
            result.getStatistics().setLastPalletStatistics(
                PalletStatistics(
                    palletId,
                    onPallet * boxVolume,
                    palletVolume));
        }

        ++palletId;

        if (onPallet == 0)
            break;
    }

    result.getStatistics().setTotalBoxes(placed);
    result.getStatistics().setFullPallets(fullPallets);

    return result;
}
