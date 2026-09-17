#pragma once

#include "IAlgorithm.h"

class RowsColsAlgorithm : public IAlgorithm
{
private:

    enum class SplitDirection
    {
        AlongLength,
        AlongWidth
    };

    struct Orientation
    {
        double length;
        double width;
        double height;

        double rotationX;
        double rotationY;
        double rotationZ;
    };

    struct Candidate
    {
        int capacity = 0;

        SplitDirection splitDirection =
            SplitDirection::AlongLength;

        // Normal-orientation section
        int normalColumns = 0;
        int normalRows = 0;

        // Rotated-orientation section
        int rotatedColumns = 0;
        int rotatedRows = 0;

        // Dimensions occupied by the complete pattern
        double usedLength = 0.0;
        double usedWidth = 0.0;

        double unusedArea = 0.0;

        Orientation normalOrientation{};
        Orientation rotatedOrientation{};
    };

    int calculateBoxesAlong(
        double palletDimension,
        double boxDimension) const;

    Candidate generateCandidate(
        const Pallet& pallet,
        const Box& box,
        const Orientation& normalOrientation,
        const Orientation& rotatedOrientation,
        int normalColumns,
        int normalRows,
        int rotatedColumns,
        int rotatedRows,
        SplitDirection splitDirection) const;

public:

    PalletizationResult generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) override;
};