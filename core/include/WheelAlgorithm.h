#pragma once

#include "IAlgorithm.h"

#include <vector>

class WheelAlgorithm : public IAlgorithm
{
private:

    /*
     * One box footprint on a 2D pallet layer.
     *
     * x, y = lower-left corner of the box
     * length, width = actual footprint after rotation
     */
    struct Footprint
    {
        double x = 0.0;
        double y = 0.0;

        double length = 0.0;
        double width = 0.0;

        double rotationZ = 0.0;
    };


    /*
     * One complete wheel layer.
     *
     * nAlongLength / nAlongWidth are the two variation
     * dimensions from the pallet-configuration document.
     * Remaining space on each edge is filled by integer
     * division with the rotated box.
     */
    struct Candidate
    {
        bool valid = false;

        int boxesPerLayer = 0;

        int nAlongLength = 0;
        int nAlongWidth = 0;

        int alongLengthBoxes = 0;
        int alongWidthBoxes = 0;

        double usedLength = 0.0;
        double usedWidth = 0.0;

        double unusedArea = 0.0;

        std::vector<Footprint> positions;
    };


    Candidate buildCandidate(
        const Pallet& pallet,
        const Box& box,
        int nAlongLength,
        int nAlongWidth,
        double alongEdge,
        double thickness,
        double horizontalRotation,
        double verticalRotation) const;


    Candidate findBestCandidate(
        const Pallet& pallet,
        const Box& box) const;


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
