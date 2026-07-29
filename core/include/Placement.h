#pragma once

class Placement
{
private:
    int boxId;

    double x;
    double y;

    double length;
    double width;

    double rotation;

public:
    Placement(int boxId,
              double x,
              double y,
              double length,
              double width,
              double rotation);

    int getBoxId() const;

    double getX() const;
    double getY() const;

    double getLength() const;
    double getWidth() const;

    double getRotation() const;
};