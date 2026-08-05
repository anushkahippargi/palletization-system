#include "Pallet.h"

Pallet::Pallet(
    double length,
    double width,
    double height,
    double maxLoadWeight)
    : length(length),
      width(width),
      height(height),
      maxLoadWeight(maxLoadWeight)
{
}

double Pallet::getLength() const
{
    return length;
}

double Pallet::getWidth() const
{
    return width;
}

double Pallet::getHeight() const
{
    return height;
}

double Pallet::getMaxLoadWeight() const
{
    return maxLoadWeight;
}