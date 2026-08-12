#include <iostream>

#include "Placement.h"

using namespace std;

Placement::Placement(
    int boxId,
    int palletId,
    const Matrix4x4& pose)
    : boxId(boxId),
      palletId(palletId),
      pose(pose)
{
}

int Placement::getBoxId() const
{
    return boxId;
}

int Placement::getPalletId() const
{
    return palletId;
}

Matrix4x4& Placement::getPose()
{
    return pose;
}

const Matrix4x4& Placement::getPose() const
{
    return pose;
}

void Placement::print() const
{
    cout << "Box ID: "
         << boxId
         << endl;

    cout << "Pallet ID: "
         << palletId
         << endl;

    cout << "Pose Matrix:"
         << endl;

    pose.print();
}