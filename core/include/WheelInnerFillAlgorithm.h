#pragma once

#include "IAlgorithm.h"

#include <vector>

/*
 * Hybrid Pinwheel
 * ----------------
 *
 * The algorithm builds a real pinwheel-style perimeter first and
 * then fills the rectangular interior with boxes in the orientation
 * that gives the best valid fill.
 *
 * Every generated placement is checked for:
 *   - pallet boundary violations
 *   - same-layer overlap
 *
 * Touching edges are allowed.
 */
class WheelInnerFillAlgorithm : public IAlgorithm
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

    struct Candidate
    {
        bool valid = false;

        int boxesPerLayer = 0;
        int perimeterBoxes = 0;
        int innerBoxes = 0;

        int topBottomBoxes = 0;
        int sideBoxes = 0;
        int innerAlongLength = 0;
        int innerAlongWidth = 0;

        double usedLength = 0.0;
        double usedWidth = 0.0;
        double unusedArea = 0.0;

        std::vector<Footprint> positions;
    };

    Candidate buildCandidate(
        const Pallet& pallet,
        const Box& box,
        int topBottomBoxes,
        int sideBoxes,
        bool innerRotated) const;

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
