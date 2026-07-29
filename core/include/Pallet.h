#pragma once

class Pallet
{
private:
    double length;
    double width;
    double maxLoadLength;
    double maxLoadWidth;

public:
    Pallet(double length,
           double width,
           double maxLoadLength,
           double maxLoadWidth);

    double getLength() const;
    double getWidth() const;
    double getMaxLoadLength() const;
    double getMaxLoadWidth() const;
};