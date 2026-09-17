#pragma once

#include "IAlgorithm.h"

class WheelInnerFillAlgorithm : public IAlgorithm
{
private:

    struct Orientation
    {
        double length;
        double width;
        double height;
        double rotationZ;
    };

    enum class InnerMode
    {
        Grid,
        Wheel
    };

    struct Candidate
    {
        bool valid = false;

        // Outer wheel
        int outerBoxesAlongLength = 0;
        int outerBoxesAlongWidth = 0;
        int outerBoxes = 0;

        // Inner arrangement
        InnerMode innerMode = InnerMode::Grid;

        int innerBoxesAlongLength = 0;
        int innerBoxesAlongWidth = 0;
        int innerBoxes = 0;

        Orientation outerOrientation{};
        Orientation innerOrientation{};

        // Total boxes in one layer
        int boxesPerLayer = 0;

        // Approximate occupied outer rectangle
        double usedLength = 0.0;
        double usedWidth = 0.0;

        double unusedArea = 0.0;
    };

    int calculateBoxesAlong(
        double palletDimension,
        double boxDimension) const;

    int calculateWheelBoxes(
        int boxesAlongLength,
        int boxesAlongWidth) const;

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

    void addOuterWheel(
        PalletizationResult& result,
        int& boxId,
        int palletId,
        double offsetX,
        double offsetY,
        double z,
        const Candidate& candidate,
        int quantity,
        bool flipLayer) const;

    void addInnerGrid(
        PalletizationResult& result,
        int& boxId,
        int palletId,
        double offsetX,
        double offsetY,
        double z,
        const Candidate& candidate,
        int quantity,
        bool flipLayer) const;

    void addInnerWheel(
        PalletizationResult& result,
        int& boxId,
        int palletId,
        double offsetX,
        double offsetY,
        double z,
        const Candidate& candidate,
        int quantity,
        bool flipLayer) const;

public:

    PalletizationResult generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) override;
};