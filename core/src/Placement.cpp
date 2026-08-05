#include <iostream>

#include "Placement.h"

using namespace std;

Placement::Placement(
    int boxId,
    const Matrix4x4& pose)
    : boxId(boxId),
      pose(pose)
{
}

int Placement::getBoxId() const
{
    return boxId;
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
    cout << "Box ID: " << boxId << endl;
    cout << "Pose Matrix:" << endl;

    pose.print();
}