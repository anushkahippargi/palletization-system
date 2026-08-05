#pragma once

#include "Matrix4x4.h"

class Placement
{
private:

    int boxId;

    Matrix4x4 pose;

public:

    Placement(
        int boxId,
        const Matrix4x4& pose);

    int getBoxId() const;

    Matrix4x4& getPose();

    const Matrix4x4& getPose() const;

    void print() const;
};