#pragma once

class Pallet
{
private:

    double length;
    double width;
    double height;

    double maxLoadWeight;

public:

    Pallet(
        double length,
        double width,
        double height,
        double maxLoadWeight);

    double getLength() const;
    double getWidth() const;
    double getHeight() const;

    double getMaxLoadWeight() const;
};