
#pragma once

#include "IAlgorithm.h"
#include <vector>

class PinwheelAlgorithm : public IAlgorithm
{
private:
    struct Footprint
    {
        double x = 0.0;
        double y = 0.0;
        double length = 0.0;
        double width = 0.0;
        double rotationZ = 0.0;
    };

    bool fitsOnPallet(
        const Footprint& footprint,
        const Pallet& pallet) const;

    bool overlaps(
        const Footprint& first,
        const Footprint& second) const;

    bool isValidLayer(
        const std::vector<Footprint>& positions,
        const Pallet& pallet) const;

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
