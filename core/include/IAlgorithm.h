#pragma once

#include "Box.h"
#include "Pallet.h"
#include "PalletizationResult.h"

class IAlgorithm
{
public:

    virtual ~IAlgorithm() = default;

    virtual PalletizationResult generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) = 0;
};