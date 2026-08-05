#pragma once

#include <vector>

#include "IAlgorithm.h"

using namespace std;

class RowAlgorithm : public IAlgorithm
{
public:

    vector<Placement> generatePattern(
        const Pallet& pallet,
        const Box& box,
        int quantity) override;
};