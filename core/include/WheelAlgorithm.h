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
     * Complete valid pinwheel layer.
     */
    struct Candidate
    {
        bool valid = false;

        int boxesPerLayer = 0;

        int unitsAlongLength = 0;
        int unitsAlongWidth = 0;

        double unitSize = 0.0;

        double usedLength = 0.0;
        double usedWidth = 0.0;

        double unusedArea = 0.0;

        std::vector<Footprint> positions;
    };


    /*
     * Build one complete pinwheel layer.
     */
    Candidate buildCandidate(
        const Pallet& pallet,
        const Box& box,
        bool reversePattern) const;


    /*
     * Check whether one box is completely inside
     * the pallet boundary.
     */
    bool fitsOnPallet(
        const Footprint& footprint,
        const Pallet& pallet) const;


    /*
     * Check whether two boxes have positive-area
     * intersection.
     *
     * Touching edges are allowed.
     */
    bool overlaps(
        const Footprint& first,
        const Footprint& second) const;


    /*
     * Validate the entire generated layer.
     */
    bool isValidLayer(
        const std::vector<Footprint>& positions,
        const Pallet& pallet) const;


    /*
     * Convert a footprint into the project's
     * existing Placement representation.
     */
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