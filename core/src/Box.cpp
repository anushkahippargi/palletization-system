#include "Box.h"

Box::Box(double length,
         double width,
         double height,
         double weight)
    : length(length),
      width(width),
      height(height),
      weight(weight) /* member initializer list to initialize the data members */ 
{/* constructor body is empty because all 
initialization is done in the member initializer list */
}

double Box::getLength() const
{
    return length;
}

double Box::getWidth() const
{
    return width;
}

double Box::getHeight() const
{
    return height;
}

double Box::getWeight() const
{
    return weight;
}