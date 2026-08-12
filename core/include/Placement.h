#pragma once

#include "Matrix4x4.h"

class Placement
{
private:

    int boxId;

    int palletId;

    Matrix4x4 pose;

public:

    Placement(
        int boxId,
        int palletId,
        const Matrix4x4& pose);

    int getBoxId() const;

    int getPalletId() const;

    Matrix4x4& getPose();

    const Matrix4x4& getPose() const;

    void print() const;
};