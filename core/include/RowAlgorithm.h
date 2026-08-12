#pragma once

#include "IAlgorithm.h"

using namespace std;

class RowAlgorithm : public IAlgorithm
{
private:

    struct Orientation
    {
        double length;
        double width;
        double height;

        double rotationX;
        double rotationY;
        double rotationZ;
    };

    int calculateBoxesAlongX(
        const Pallet& pallet,
        double boxLength) const;

    int calculateBoxesAlongY(
        const Pallet& pallet,
        double boxWidth) const;

    int calculateBoxesAlongZ(
        const Pallet& pallet,
        double boxHeight) const;

    Orientation findBestOrientation(
        const Pallet& pallet,
        const Box& box) const;

public:

    PalletizationResult generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) override;
};