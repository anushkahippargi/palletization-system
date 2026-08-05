#pragma once

#include <vector>

#include "Box.h"
#include "Pallet.h"
#include "Placement.h"

using namespace std;

class IAlgorithm
{
public:

    virtual ~IAlgorithm() = default;

    virtual vector<Placement> generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) = 0;
};