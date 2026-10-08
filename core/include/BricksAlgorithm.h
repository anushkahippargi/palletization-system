#pragma once

#include "IAlgorithm.h"

class BricksAlgorithm : public IAlgorithm
{
public:
    PalletizationResult generatePattern(const Pallet& pallet,
                                        const Box& box,
                                        int quantity) override;
};