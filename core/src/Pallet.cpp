# include "Pallet.h"
Pallet::Pallet(double length,
               double width,
               double maxLoadLength,
               double maxLoadWidth)
    : length(length),
      width(width),
      maxLoadLength(maxLoadLength),
      maxLoadWidth(maxLoadWidth) {}

double Pallet::getLength() const
{
    return length;
}   

double Pallet::getWidth() const
{
    return width;
}
double Pallet::getMaxLoadLength() const
{
    return maxLoadLength;
}
double Pallet::getMaxLoadWidth() const
{
    return maxLoadWidth;
}
