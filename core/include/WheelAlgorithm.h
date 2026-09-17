#pragma once

#include "IAlgorithm.h"

class WheelAlgorithm : public IAlgorithm
{
private:

    struct Orientation
    {
        double length;
        double width;
        double height;

        double rotationZ;
    };

    struct Candidate
    {
        bool valid = false;

        // Number of boxes along the pallet length
        int boxesAlongLength = 0;

        // Number of boxes along the pallet width
        int boxesAlongWidth = 0;

        // Number of boxes in one complete wheel layer
        int boxesPerLayer = 0;

        // Dimensions occupied by the wheel
        double usedLength = 0.0;
        double usedWidth = 0.0;

        // Unused pallet area
        double unusedArea = 0.0;

        // Box orientation used by this candidate
        Orientation orientation{};
    };

    int calculateBoxesAlong(
        double palletDimension,
        double boxDimension) const;

    Candidate evaluateCandidate(
        const Pallet& pallet,
        const Orientation& orientation,
        int boxesAlongLength,
        int boxesAlongWidth) const;

    void addPlacement(
        PalletizationResult& result,
        int boxId,
        int palletId,
        double x,
        double y,
        double z,
        double rotationZ) const;

public:

    PalletizationResult generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) override;
};