#include <cmath>
#include <iostream>

#include "Matrix4x4.h"

using namespace std;

Matrix4x4::Matrix4x4()
{
    setIdentity();
}

void Matrix4x4::setIdentity()
{
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            matrix[row][column] =
                (row == column) ? 1.0 : 0.0;
        }
    }
}

void Matrix4x4::setTranslation(
    double x,
    double y,
    double z)
{
    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;
}

void Matrix4x4::setRotationX(
    double angleDegrees)
{
    double angleRadians =
        angleDegrees * M_PI / 180.0;

    double cosine =
        cos(angleRadians);

    double sine =
        sin(angleRadians);

    matrix[0][0] = 1.0;
    matrix[0][1] = 0.0;
    matrix[0][2] = 0.0;

    matrix[1][0] = 0.0;
    matrix[1][1] = cosine;
    matrix[1][2] = -sine;

    matrix[2][0] = 0.0;
    matrix[2][1] = sine;
    matrix[2][2] = cosine;
}

void Matrix4x4::setRotationY(
    double angleDegrees)
{
    double angleRadians =
        angleDegrees * M_PI / 180.0;

    double cosine =
        cos(angleRadians);

    double sine =
        sin(angleRadians);

    matrix[0][0] = cosine;
    matrix[0][1] = 0.0;
    matrix[0][2] = sine;

    matrix[1][0] = 0.0;
    matrix[1][1] = 1.0;
    matrix[1][2] = 0.0;

    matrix[2][0] = -sine;
    matrix[2][1] = 0.0;
    matrix[2][2] = cosine;
}

void Matrix4x4::setRotationZ(
    double angleDegrees)
{
    double angleRadians =
        angleDegrees * M_PI / 180.0;

    double cosine =
        cos(angleRadians);

    double sine =
        sin(angleRadians);

    matrix[0][0] = cosine;
    matrix[0][1] = -sine;
    matrix[0][2] = 0.0;

    matrix[1][0] = sine;
    matrix[1][1] = cosine;
    matrix[1][2] = 0.0;

    matrix[2][0] = 0.0;
    matrix[2][1] = 0.0;
    matrix[2][2] = 1.0;
}

void Matrix4x4::print() const
{
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            cout << matrix[row][column]
                 << " ";
        }

        cout << endl;
    }
}