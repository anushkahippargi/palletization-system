#include "Placement.h"
Placement::Placement(int boxId,
                     double x,
                     double y,
                     double length,
                     double width,
                     double rotation)
    : boxId(boxId),
      x(x),
      y(y),
      length(length),
      width(width),
      rotation(rotation) {}

int Placement::getBoxId() const
{
    return boxId;
}

double Placement::getX() const
{
    return x;
}

double Placement::getY() const
{
    return y;
}

double Placement::getLength() const
{
    return length;
}

double Placement::getWidth() const
{
    return width;
}

double Placement::getRotation() const
{
    return rotation;
}
