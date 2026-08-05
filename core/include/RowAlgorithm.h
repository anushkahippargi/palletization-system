#pragma once

#include "IAlgorithm.h"

class RowAlgorithm : public IAlgorithm
{
public:

    PalletizationResult generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) override;
};