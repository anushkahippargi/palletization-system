#pragma once

#include "IAlgorithm.h"

class BricksAlgorithm : public IAlgorithm
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

        int boxesAlongLength = 0;
        int boxesAlongWidth = 0;

        int boxesPerLayer = 0;

        double usedLength = 0.0;
        double usedWidth = 0.0;

        double unusedArea = 0.0;

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