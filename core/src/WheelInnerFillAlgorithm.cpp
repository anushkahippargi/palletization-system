#include "WheelInnerFillAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    constexpr double EPSILON = 1e-9;
}

int WheelInnerFillAlgorithm::calculateBoxesAlong(
    double palletDimension,
    double boxDimension) const
{
    if (boxDimension <= EPSILON)
    {
        return 0;
    }

    return static_cast<int>(
        std::floor(
            (palletDimension + EPSILON) /
            boxDimension
        )
    );
}

int WheelInnerFillAlgorithm::calculateWheelBoxes(
    int boxesAlongLength,
    int boxesAlongWidth) const
{
    if (boxesAlongLength < 2 ||
        boxesAlongWidth < 2)
    {
        return 0;
    }

    /*
     * Number of boxes on the perimeter:
     *
     * top    = boxesAlongLength
     * bottom = boxesAlongLength
     * left   = boxesAlongWidth - 2
     * right  = boxesAlongWidth - 2
     */
    return
        (2 * boxesAlongLength) +
        (2 * (boxesAlongWidth - 2));
}

WheelInnerFillAlgorithm::Candidate
WheelInnerFillAlgorithm::evaluateCandidate(
    const Pallet& pallet,
    const Orientation& orientation,
    int boxesAlongLength,
    int boxesAlongWidth) const
{
    Candidate candidate;

    if (boxesAlongLength < 2 ||
        boxesAlongWidth < 2)
    {
        return candidate;
    }

    const double outerLength =
        boxesAlongLength *
        orientation.length;

    const double outerWidth =
        boxesAlongWidth *
        orientation.width;

    /*
     * The complete outer wheel must fit on the pallet.
     */
    if (outerLength >
            pallet.getLength() + EPSILON ||
        outerWidth >
            pallet.getWidth() + EPSILON)
    {
        return candidate;
    }

    const int outerBoxes =
        calculateWheelBoxes(
            boxesAlongLength,
            boxesAlongWidth
        );

    if (outerBoxes <= 0)
    {
        return candidate;
    }

    candidate.valid = true;

    candidate.outerBoxesAlongLength =
        boxesAlongLength;

    candidate.outerBoxesAlongWidth =
        boxesAlongWidth;

    candidate.outerBoxes =
        outerBoxes;

    candidate.outerOrientation =
        orientation;

    candidate.usedLength =
        outerLength;

    candidate.usedWidth =
        outerWidth;

    /*
     * ============================================================
     * ACTUAL INTERIOR AREA
     * ============================================================
     *
     * The side boxes are rotated by 90 degrees.
     *
     * Therefore the safe interior starts after the width of the
     * side boxes and ends before the opposite side.
     *
     * This is calculated for EVERY possible outer wheel.
     */
    const double innerStartX =
        orientation.width;

    const double innerStartY =
        orientation.width;

    const double innerEndX =
        (boxesAlongLength - 1) *
        orientation.length;

    const double innerEndY =
        (boxesAlongWidth - 1) *
        orientation.width;

    const double innerLength =
        innerEndX -
        innerStartX;

    const double innerWidth =
        innerEndY -
        innerStartY;

    if (innerLength <= EPSILON ||
        innerWidth <= EPSILON)
    {
        candidate.boxesPerLayer =
            outerBoxes;

        candidate.unusedArea =
            std::max(
                0.0,
                pallet.getLength() *
                    pallet.getWidth() -
                outerLength *
                    outerWidth
            );

        return candidate;
    }

    /*
     * We will test:
     *
     * 1. Normal grid inside the wheel
     * 2. Rotated grid inside the wheel
     * 3. Wheel inside the wheel
     * 4. Rotated wheel inside the wheel
     */
    Orientation innerOrientations[2] =
    {
        {
            orientation.length,
            orientation.width,
            orientation.height,
            orientation.rotationZ
        },

        {
            orientation.width,
            orientation.length,
            orientation.height,
            orientation.rotationZ + 90.0
        }
    };

    int bestInnerBoxes = 0;

    InnerMode bestMode =
        InnerMode::Grid;

    int bestInnerLengthCount = 0;
    int bestInnerWidthCount = 0;

    Orientation bestInnerOrientation{};

    /*
     * ============================================================
     * OPTION 1: NORMAL GRID FILL
     * ============================================================
     */
    for (const Orientation& innerOrientation :
         innerOrientations)
    {
        const int countLength =
            calculateBoxesAlong(
                innerLength,
                innerOrientation.length
            );

        const int countWidth =
            calculateBoxesAlong(
                innerWidth,
                innerOrientation.width
            );

        const int numberOfBoxes =
            countLength *
            countWidth;

        if (numberOfBoxes >
            bestInnerBoxes)
        {
            bestInnerBoxes =
                numberOfBoxes;

            bestMode =
                InnerMode::Grid;

            bestInnerLengthCount =
                countLength;

            bestInnerWidthCount =
                countWidth;

            bestInnerOrientation =
                innerOrientation;
        }
    }

    /*
     * ============================================================
     * OPTION 2: WHEEL INSIDE THE WHEEL
     * ============================================================
     */
    for (const Orientation& innerOrientation :
         innerOrientations)
    {
        const int maxLength =
            calculateBoxesAlong(
                innerLength,
                innerOrientation.length
            );

        const int maxWidth =
            calculateBoxesAlong(
                innerWidth,
                innerOrientation.width
            );

        for (int innerL = 2;
             innerL <= maxLength;
             ++innerL)
        {
            for (int innerW = 2;
                 innerW <= maxWidth;
                 ++innerW)
            {
                const double requiredLength =
                    innerL *
                    innerOrientation.length;

                const double requiredWidth =
                    innerW *
                    innerOrientation.width;

                if (requiredLength >
                        innerLength + EPSILON ||
                    requiredWidth >
                        innerWidth + EPSILON)
                {
                    continue;
                }

                const int wheelBoxes =
                    calculateWheelBoxes(
                        innerL,
                        innerW
                    );

                if (wheelBoxes >
                    bestInnerBoxes)
                {
                    bestInnerBoxes =
                        wheelBoxes;

                    bestMode =
                        InnerMode::Wheel;

                    bestInnerLengthCount =
                        innerL;

                    bestInnerWidthCount =
                        innerW;

                    bestInnerOrientation =
                        innerOrientation;
                }
            }
        }
    }

    candidate.innerMode =
        bestMode;

    candidate.innerBoxesAlongLength =
        bestInnerLengthCount;

    candidate.innerBoxesAlongWidth =
        bestInnerWidthCount;

    candidate.innerBoxes =
        bestInnerBoxes;

    candidate.innerOrientation =
        bestInnerOrientation;

    candidate.boxesPerLayer =
        candidate.outerBoxes +
        candidate.innerBoxes;

    /*
     * ============================================================
     * SCORE
     * ============================================================
     *
     * Primary:
     *   More boxes per layer.
     *
     * Secondary:
     *   Less unused pallet area.
     */
    const double palletArea =
        pallet.getLength() *
        pallet.getWidth();

    const double usedArea =
        candidate.usedLength *
        candidate.usedWidth;

    candidate.unusedArea =
        std::max(
            0.0,
            palletArea - usedArea
        );

    return candidate;
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

    result.getPlacements().push_back(
        placement
    );
}

void WheelInnerFillAlgorithm::addOuterWheel(
    PalletizationResult& result,
    int& boxId,
    int palletId,
    double offsetX,
    double offsetY,
    double z,
    const Candidate& candidate,
    int maxBoxId,
    bool flipLayer) const
{
    const Orientation& o =
        candidate.outerOrientation;

    const int nL =
        candidate.outerBoxesAlongLength;

    const int nW =
        candidate.outerBoxesAlongWidth;

    const double layerRotation =
        flipLayer
            ? 180.0
            : 0.0;

    auto place =
        [&](double x,
            double y,
            double rotation)
        {
            if (boxId > maxBoxId)
            {
                return;
            }

            addPlacement(
                result,
                boxId,
                palletId,
                x,
                y,
                z,
                rotation +
                    layerRotation
            );

            ++boxId;
        };

    /*
     * TOP
     */
    for (int i = 0;
         i < nL;
         ++i)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            offsetX +
                i * o.length,
            offsetY,
            o.rotationZ
        );
    }

    /*
     * BOTTOM
     */
    for (int i = 0;
         i < nL;
         ++i)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            offsetX +
                i * o.length,
            offsetY +
                (nW - 1) *
                    o.width,
            o.rotationZ
        );
    }

    /*
     * LEFT
     */
    for (int j = 1;
         j < nW - 1;
         ++j)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            offsetX,
            offsetY +
                j * o.width,
            o.rotationZ + 90.0
        );
    }

    /*
     * RIGHT
     */
    for (int j = 1;
         j < nW - 1;
         ++j)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            offsetX +
                (nL - 1) *
                    o.length,
            offsetY +
                j * o.width,
            o.rotationZ + 90.0
        );
    }
}

void WheelInnerFillAlgorithm::addInnerGrid(
    PalletizationResult& result,
    int& boxId,
    int palletId,
    double offsetX,
    double offsetY,
    double z,
    const Candidate& candidate,
    int maxBoxId,
    bool flipLayer) const
{
    if (candidate.innerBoxes <= 0)
    {
        return;
    }

    const Orientation& outer =
        candidate.outerOrientation;

    const Orientation& inner =
        candidate.innerOrientation;

    /*
     * Safe interior starting point.
     */
    const double startX =
        offsetX +
        outer.width;

    const double startY =
        offsetY +
        outer.width;

    const double layerRotation =
        flipLayer
            ? 180.0
            : 0.0;

    for (int row = 0;
         row <
             candidate.innerBoxesAlongWidth;
         ++row)
    {
        for (int col = 0;
             col <
                 candidate.innerBoxesAlongLength;
             ++col)
        {
            if (boxId > maxBoxId)
            {
                return;
            }

            const double x =
                startX +
                col *
                    inner.length;

            const double y =
                startY +
                row *
                    inner.width;

            addPlacement(
                result,
                boxId,
                palletId,
                x,
                y,
                z,
                inner.rotationZ +
                    layerRotation
            );

            ++boxId;
        }
    }
}

void WheelInnerFillAlgorithm::addInnerWheel(
    PalletizationResult& result,
    int& boxId,
    int palletId,
    double offsetX,
    double offsetY,
    double z,
    const Candidate& candidate,
    int maxBoxId,
    bool flipLayer) const
{
    if (candidate.innerBoxes <= 0)
    {
        return;
    }

    const Orientation& outer =
        candidate.outerOrientation;

    const Orientation& inner =
        candidate.innerOrientation;

    const int nL =
        candidate.innerBoxesAlongLength;

    const int nW =
        candidate.innerBoxesAlongWidth;

    /*
     * Available interior dimensions.
     */
    const double availableLength =
        (candidate.outerBoxesAlongLength - 1) *
            outer.length -
        outer.width;

    const double availableWidth =
        (candidate.outerBoxesAlongWidth - 2) *
            outer.width;

    /*
     * Dimensions of the inner wheel.
     */
    const double wheelLength =
        nL *
        inner.length;

    const double wheelWidth =
        nW *
        inner.width;

    /*
     * Center the inner wheel.
     */
    const double startX =
        offsetX +
        outer.width +
        std::max(
            0.0,
            (availableLength -
             wheelLength) / 2.0
        );

    const double startY =
        offsetY +
        outer.width +
        std::max(
            0.0,
            (availableWidth -
             wheelWidth) / 2.0
        );

    const double layerRotation =
        flipLayer
            ? 180.0
            : 0.0;

    auto place =
        [&](double x,
            double y,
            double rotation)
        {
            if (boxId > maxBoxId)
            {
                return;
            }

            addPlacement(
                result,
                boxId,
                palletId,
                x,
                y,
                z,
                rotation +
                    layerRotation
            );

            ++boxId;
        };

    /*
     * TOP
     */
    for (int i = 0;
         i < nL;
         ++i)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            startX +
                i * inner.length,
            startY,
            inner.rotationZ
        );
    }

    /*
     * BOTTOM
     */
    for (int i = 0;
         i < nL;
         ++i)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            startX +
                i * inner.length,
            startY +
                (nW - 1) *
                    inner.width,
            inner.rotationZ
        );
    }

    /*
     * LEFT
     */
    for (int j = 1;
         j < nW - 1;
         ++j)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            startX,
            startY +
                j * inner.width,
            inner.rotationZ + 90.0
        );
    }

    /*
     * RIGHT
     */
    for (int j = 1;
         j < nW - 1;
         ++j)
    {
        if (boxId > maxBoxId)
        {
            return;
        }

        place(
            startX +
                (nL - 1) *
                    inner.length,
            startY +
                j * inner.width,
            inner.rotationZ + 90.0
        );
    }
}

PalletizationResult
WheelInnerFillAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;

    if (quantity <= 0)
    {
        return result;
    }

    const double boxLength =
        box.getLength();

    const double boxWidth =
        box.getWidth();

    const double boxHeight =
        box.getHeight();

    if (boxLength <= EPSILON ||
        boxWidth <= EPSILON ||
        boxHeight <= EPSILON)
    {
        return result;
    }

    /*
     * ============================================================
     * TWO BASIC BOX ORIENTATIONS
     * ============================================================
     *
     * Orientation 1:
     *   L x W
     *
     * Orientation 2:
     *   W x L
     */
    Orientation orientations[2] =
    {
        {
            boxLength,
            boxWidth,
            boxHeight,
            0.0
        },

        {
            boxWidth,
            boxLength,
            boxHeight,
            90.0
        }
    };

    Candidate bestCandidate;

    /*
     * ============================================================
     * SEARCH ALL POSSIBLE OUTER WHEELS
     * ============================================================
     */
    for (const Orientation& orientation :
         orientations)
    {
        const int maxL =
            calculateBoxesAlong(
                pallet.getLength(),
                orientation.length
            );

        const int maxW =
            calculateBoxesAlong(
                pallet.getWidth(),
                orientation.width
            );

        for (int nL = 2;
             nL <= maxL;
             ++nL)
        {
            for (int nW = 2;
                 nW <= maxW;
                 ++nW)
            {
                Candidate candidate =
                    evaluateCandidate(
                        pallet,
                        orientation,
                        nL,
                        nW
                    );

                if (!candidate.valid)
                {
                    continue;
                }

                /*
                 * Choose the arrangement which puts the most
                 * boxes on one layer.
                 *
                 * If equal, choose the one occupying less
                 * unused pallet area.
                 */
                if (!bestCandidate.valid ||
                    candidate.boxesPerLayer >
                        bestCandidate.boxesPerLayer ||
                    (
                        candidate.boxesPerLayer ==
                            bestCandidate.boxesPerLayer &&
                        candidate.unusedArea <
                            bestCandidate.unusedArea
                    ))
                {
                    bestCandidate =
                        candidate;
                }
            }
        }
    }

    if (!bestCandidate.valid ||
        bestCandidate.boxesPerLayer <= 0)
    {
        return result;
    }

    /*
     * ============================================================
     * NUMBER OF LAYERS
     * ============================================================
     */
    const int layers =
        static_cast<int>(
            std::floor(
                (pallet.getHeight() +
                 EPSILON) /
                boxHeight
            )
        );

    if (layers <= 0)
    {
        return result;
    }

    /*
     * Maximum boxes that this arrangement can hold on one pallet.
     */
    const int boxesPerPallet =
        bestCandidate.boxesPerLayer *
        layers;

    const int palletCount =
        static_cast<int>(
            std::ceil(
                static_cast<double>(quantity) /
                static_cast<double>(
                    boxesPerPallet
                )
            )
        );

    int boxId = 1;

    int fullPallets = 0;

    /*
     * ============================================================
     * GENERATE PALLETS
     * ============================================================
     */
    for (int palletId = 1;
         palletId <= palletCount;
         ++palletId)
    {
        const int remainingBoxes =
            quantity -
            (boxId - 1);

        if (remainingBoxes <= 0)
        {
            break;
        }

        const int boxesOnThisPallet =
            std::min(
                boxesPerPallet,
                remainingBoxes
            );

        /*
         * Center the arrangement on the pallet.
         */
        const double offsetX =
            std::max(
                0.0,
                (
                    pallet.getLength() -
                    bestCandidate.usedLength
                ) / 2.0
            );

        const double offsetY =
            std::max(
                0.0,
                (
                    pallet.getWidth() -
                    bestCandidate.usedWidth
                ) / 2.0
            );

        const int palletStartBoxId =
            boxId;

        /*
         * ========================================================
         * LAYERS
         * ========================================================
         */
        for (int layer = 0;
             layer < layers;
             ++layer)
        {
            if (boxId >
                palletStartBoxId +
                    boxesOnThisPallet -
                    1)
            {
                break;
            }

            const int layerStartBoxId =
                boxId;

            const int maxBoxId =
                palletStartBoxId +
                boxesOnThisPallet -
                1;

            const bool flipLayer =
                (layer % 2 == 1);

            const double z =
                layer *
                boxHeight;

            /*
             * First place the OUTER WHEEL.
             */
            addOuterWheel(
                result,
                boxId,
                palletId,
                offsetX,
                offsetY,
                z,
                bestCandidate,
                maxBoxId,
                flipLayer
            );

            /*
             * Then use whatever box capacity remains in the
             * interior.
             */
            if (boxId <= maxBoxId)
            {
                if (bestCandidate.innerMode ==
                    InnerMode::Grid)
                {
                    addInnerGrid(
                        result,
                        boxId,
                        palletId,
                        offsetX,
                        offsetY,
                        z,
                        bestCandidate,
                        maxBoxId,
                        flipLayer
                    );
                }
                else
                {
                    addInnerWheel(
                        result,
                        boxId,
                        palletId,
                        offsetX,
                        offsetY,
                        z,
                        bestCandidate,
                        maxBoxId,
                        flipLayer
                    );
                }
            }

            /*
             * Safety check: every layer must make progress.
             */
            if (boxId ==
                layerStartBoxId)
            {
                break;
            }
        }

        const int boxesPlacedOnPallet =
            boxId -
            palletStartBoxId;

        if (boxesPlacedOnPallet ==
            boxesPerPallet)
        {
            ++fullPallets;
        }
    }

    /*
     * ============================================================
     * STATISTICS
     * ============================================================
     */
    result.getStatistics().setTotalBoxes(
        quantity
    );

    result.getStatistics().setFullPallets(
        fullPallets
    );

    /*
     * Last pallet statistics.
     */
    const int boxesOnLastPallet =
        quantity -
        (
            (palletCount - 1) *
            boxesPerPallet
        );

    const double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();

    const double boxVolume =
        boxLength *
        boxWidth *
        boxHeight;

    const double usedVolume =
        boxesOnLastPallet *
        boxVolume;

    result.getStatistics().setLastPalletStatistics(
        PalletStatistics(
            palletCount,
            usedVolume,
            palletVolume
        )
    );

    return result;
}