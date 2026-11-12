
#include "WheelAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
    constexpr double EPSILON = 1e-6;

    double normalizeAngle(double angle)
    {
        while (angle >= 360.0)
            angle -= 360.0;

        while (angle < 0.0)
            angle += 360.0;

        return angle;
    }
}

// ------------------------------------------------------------
// BUILD ONE CLOCKWISE WHEEL PERIMETER
// ------------------------------------------------------------

WheelAlgorithm::Candidate WheelAlgorithm::buildCandidate(
    const Pallet& pallet,
    const Box& box,
    int nAlongLength,
    int nAlongWidth,
    double alongEdge,
    double thickness,
    double horizontalRotation,
    double verticalRotation) const
{
    Candidate candidate;

    const double palletLength = pallet.getLength();
    const double palletWidth = pallet.getWidth();

    if (palletLength <= 0.0 ||
        palletWidth <= 0.0 ||
        alongEdge <= 0.0 ||
        thickness <= 0.0 ||
        nAlongLength <= 0 ||
        nAlongWidth <= 0)
    {
        return candidate;
    }

    const double horizontalUsed =
        nAlongLength * alongEdge;

    const double verticalUsed =
        nAlongWidth * alongEdge;

    if (horizontalUsed > palletLength + EPSILON ||
        verticalUsed >
            palletWidth - 2.0 * thickness + EPSILON)
    {
        return candidate;
    }

    const double left = 0.0;
    const double right = palletLength - thickness;
    const double bottom = 0.0;
    const double top = palletWidth - thickness;

    // 1. TOP EDGE: left to right.
    for (int i = 0; i < nAlongLength; ++i)
    {
        const double x = i * alongEdge;

        candidate.positions.push_back(
            {
                x,
                top,
                alongEdge,
                thickness,
                normalizeAngle(horizontalRotation)
            });
    }

    // 2. RIGHT EDGE: top to bottom.
    for (int i = 0; i < nAlongWidth; ++i)
    {
        const double y = top - (i + 1) * alongEdge;

        candidate.positions.push_back(
            {
                right,
                y,
                thickness,
                alongEdge,
                normalizeAngle(verticalRotation)
            });
    }

    // 3. BOTTOM EDGE: right to left.
    for (int i = nAlongLength - 1; i >= 0; --i)
    {
        const double x = i * alongEdge;

        candidate.positions.push_back(
            {
                x,
                bottom,
                alongEdge,
                thickness,
                normalizeAngle(horizontalRotation)
            });
    }

    // 4. LEFT EDGE: bottom to top.
    for (int i = 0; i < nAlongWidth; ++i)
    {
        const double y =
            bottom + thickness + i * alongEdge;

        candidate.positions.push_back(
            {
                left,
                y,
                thickness,
                alongEdge,
                normalizeAngle(verticalRotation)
            });
    }

    if (!isValidLayer(candidate.positions, pallet))
    {
        candidate.positions.clear();
        return candidate;
    }

    candidate.valid = true;

    candidate.boxesPerLayer =
        static_cast<int>(candidate.positions.size());

    candidate.nAlongLength = nAlongLength;
    candidate.nAlongWidth = nAlongWidth;

    candidate.alongLengthBoxes = nAlongLength;
    candidate.alongWidthBoxes = nAlongWidth;

    candidate.usedLength = palletLength;
    candidate.usedWidth = palletWidth;

    candidate.unusedArea =
        palletLength * palletWidth -
        candidate.boxesPerLayer *
            box.getLength() * box.getWidth();

    return candidate;
}

// ------------------------------------------------------------
// FIND THE BEST VALID PERIMETER
// ------------------------------------------------------------

WheelAlgorithm::Candidate WheelAlgorithm::findBestCandidate(
    const Pallet& pallet,
    const Box& box) const
{
    Candidate best;

    const double boxLength = box.getLength();
    const double boxWidth = box.getWidth();

    if (boxLength <= 0.0 || boxWidth <= 0.0)
        return best;

    // Evaluate both possible box orientations.
    for (int orientation = 0; orientation < 2; ++orientation)
    {
        const double alongEdge =
            orientation == 0 ? boxLength : boxWidth;

        const double thickness =
            orientation == 0 ? boxWidth : boxLength;

        const double horizontalRotation =
            orientation == 0 ? 0.0 : 90.0;

        const double verticalRotation =
            normalizeAngle(horizontalRotation + 90.0);

        const double middleWidth =
            pallet.getWidth() - 2.0 * thickness;

        if (middleWidth < alongEdge - EPSILON)
            continue;

        const int maxAlongLength =
            static_cast<int>(
                std::floor(
                    (pallet.getLength() + EPSILON) /
                    alongEdge));

        const int maxAlongWidth =
            static_cast<int>(
                std::floor(
                    (middleWidth + EPSILON) /
                    alongEdge));

        for (int nLength = 1;
             nLength <= maxAlongLength;
             ++nLength)
        {
            for (int nWidth = 1;
                 nWidth <= maxAlongWidth;
                 ++nWidth)
            {
                Candidate candidate = buildCandidate(
                    pallet,
                    box,
                    nLength,
                    nWidth,
                    alongEdge,
                    thickness,
                    horizontalRotation,
                    verticalRotation);

                if (!candidate.valid)
                    continue;

                if (!best.valid ||
                    candidate.boxesPerLayer >
                        best.boxesPerLayer ||
                    (candidate.boxesPerLayer ==
                         best.boxesPerLayer &&
                     candidate.unusedArea <
                         best.unusedArea - EPSILON))
                {
                    best = candidate;
                }
            }
        }
    }

    return best;
}

// ------------------------------------------------------------
// GEOMETRY VALIDATION
// ------------------------------------------------------------

bool WheelAlgorithm::fitsOnPallet(
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

bool WheelAlgorithm::overlaps(
    const Footprint& first,
    const Footprint& second) const
{
    const double overlapX =
        std::min(
            first.x + first.length,
            second.x + second.length) -
        std::max(first.x, second.x);

    const double overlapY =
        std::min(
            first.y + first.width,
            second.y + second.width) -
        std::max(first.y, second.y);

    return overlapX > EPSILON &&
           overlapY > EPSILON;
}

bool WheelAlgorithm::isValidLayer(
    const std::vector<Footprint>& positions,
    const Pallet& pallet) const
{
    if (positions.empty())
        return false;

    for (const Footprint& p : positions)
    {
        if (!fitsOnPallet(p, pallet))
            return false;
    }

    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        for (std::size_t j = i + 1;
             j < positions.size();
             ++j)
        {
            if (overlaps(positions[i], positions[j]))
                return false;
        }
    }

    return true;
}

// ------------------------------------------------------------
// ADD ONE BOX PLACEMENT
// ------------------------------------------------------------

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

    // x and y are lower-left footprint coordinates.
    // The viewer rotates the box around its centre.
    pose.setTranslation(x, y, z);
    pose.setRotationZ(rotationZ);

    result.getPlacements().emplace_back(
        boxId,
        palletId,
        pose);
}

// ------------------------------------------------------------
// GENERATE COMPLETE WHEEL PALLETIZATION
// ------------------------------------------------------------

PalletizationResult WheelAlgorithm::generatePattern(
    const Pallet& pallet,
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

    // --------------------------------------------------------
    // FIRST LAYER: ORIGINAL CLOCKWISE WHEEL
    // --------------------------------------------------------

    const Candidate firstLayer =
        findBestCandidate(pallet, box);

    if (!firstLayer.valid)
    {
        std::cout
            << "WHEEL: No valid wheel perimeter fits "
            << "these dimensions.\n";

        return result;
    }

    // --------------------------------------------------------
    // SECOND LAYER: TRY SWAPPED BOX ORIENTATION
    // --------------------------------------------------------

    const Footprint& reference =
        firstLayer.positions.front();

    const double alternateAlongEdge =
        reference.width;

    const double alternateThickness =
        reference.length;

    const double alternateHorizontalRotation =
        normalizeAngle(reference.rotationZ + 90.0);

    const double alternateVerticalRotation =
        normalizeAngle(
            alternateHorizontalRotation + 90.0);

    Candidate alternateLayer;

    const double alternateMiddleWidth =
        pallet.getWidth() -
        2.0 * alternateThickness;

    if (alternateAlongEdge > 0.0 &&
        alternateThickness > 0.0 &&
        alternateMiddleWidth >=
            alternateAlongEdge - EPSILON)
    {
        const int alternateNLength =
            static_cast<int>(
                std::floor(
                    (pallet.getLength() + EPSILON) /
                    alternateAlongEdge));

        const int alternateNWidth =
            static_cast<int>(
                std::floor(
                    (alternateMiddleWidth + EPSILON) /
                    alternateAlongEdge));

        if (alternateNLength > 0 &&
            alternateNWidth > 0)
        {
            alternateLayer = buildCandidate(
                pallet,
                box,
                alternateNLength,
                alternateNWidth,
                alternateAlongEdge,
                alternateThickness,
                alternateHorizontalRotation,
                alternateVerticalRotation);
        }
    }

    // --------------------------------------------------------
    // FULL FOOTPRINT SUPPORT VALIDATION
    // --------------------------------------------------------

    const auto isFullySupported =
        [](const std::vector<Footprint>& upper,
           const std::vector<Footprint>& lower) -> bool
    {
        for (const Footprint& u : upper)
        {
            double coveredArea = 0.0;

            for (const Footprint& l : lower)
            {
                const double overlapX =
                    std::max(
                        0.0,
                        std::min(
                            u.x + u.length,
                            l.x + l.length) -
                        std::max(u.x, l.x));

                const double overlapY =
                    std::max(
                        0.0,
                        std::min(
                            u.y + u.width,
                            l.y + l.width) -
                        std::max(u.y, l.y));

                coveredArea += overlapX * overlapY;
            }

            const double upperArea =
                u.length * u.width;

            if (coveredArea < upperArea - EPSILON)
                return false;
        }

        return true;
    };

    const bool useAlternateLayer =
        alternateLayer.valid &&
        alternateLayer.boxesPerLayer > 0 &&
        isFullySupported(
            alternateLayer.positions,
            firstLayer.positions);

    // Safe fallback if the alternate pattern is invalid.
    const Candidate& secondLayer =
        useAlternateLayer
            ? alternateLayer
            : firstLayer;

    // --------------------------------------------------------
    // PALLET CAPACITY
    // --------------------------------------------------------

    const int layers = static_cast<int>(
        std::floor(
            (pallet.getHeight() + EPSILON) /
            box.getHeight()));

    if (layers <= 0)
        return result;

    const int boxesPerPallet =
        firstLayer.boxesPerLayer +
        (layers - 1) * secondLayer.boxesPerLayer;

    const double boxVolume =
        box.getLength() *
        box.getWidth() *
        box.getHeight();

    const double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();

    std::cout
        << "\n========== WHEEL ALGORITHM ==========\n"
        << "First layer boxes: "
        << firstLayer.boxesPerLayer << '\n'
        << "Second layer boxes: "
        << secondLayer.boxesPerLayer << '\n'
        << "Alternate orientation used: "
        << (useAlternateLayer ? "YES" : "NO - SAFE FALLBACK")
        << '\n'
        << "Layers per pallet: "
        << layers << '\n'
        << "Boxes per full pallet: "
        << boxesPerPallet << '\n'
        << "=====================================\n";

    // --------------------------------------------------------
    // PLACE BOXES ON PALLETS
    // --------------------------------------------------------

    int boxesPlaced = 0;
    int fullPallets = 0;
    int palletId = 1;

    while (boxesPlaced < quantity)
    {
        const int boxesBeforePallet = boxesPlaced;

        for (int layer = 0;
             layer < layers && boxesPlaced < quantity;
             ++layer)
        {
            const double z =
                layer * box.getHeight();

            const Candidate& pattern =
                (layer == 0)
                    ? firstLayer
                    : secondLayer;

            for (const Footprint& p : pattern.positions)
            {
                if (boxesPlaced >= quantity)
                    break;

                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    p.x,
                    p.y,
                    z,
                    p.rotationZ);

                ++boxesPlaced;
            }
        }

        const int boxesOnPallet =
            boxesPlaced - boxesBeforePallet;

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

        if (boxesOnPallet == 0)
            break;

        ++palletId;
    }

    result.getStatistics().setTotalBoxes(boxesPlaced);
    result.getStatistics().setFullPallets(fullPallets);

    std::cout
        << "Boxes requested: " << quantity << '\n'
        << "Boxes placed: " << boxesPlaced << '\n'
        << "Full pallets: " << fullPallets << '\n';

    return result;
}
