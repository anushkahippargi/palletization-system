#pragma once


class Matrix4x4
{
private:

    double transform[4][4];

public:

    Matrix4x4();

    void setIdentity();

    void setTranslation(
        double x,
        double y,
        double z);

    void setRotationZ(
        double angleDegrees);

    double getValue(
        int row,
        int column) const;

    void print() const;
};