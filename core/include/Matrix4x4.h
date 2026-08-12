#pragma once

class Matrix4x4
{
private:

    double matrix[4][4];

public:

    Matrix4x4();

    void setIdentity();

    void setTranslation(
        double x,
        double y,
        double z);

    void setRotationX(
        double angleDegrees);

    void setRotationY(
        double angleDegrees);

    void setRotationZ(
        double angleDegrees);

    void print() const;
};