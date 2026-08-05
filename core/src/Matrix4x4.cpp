#include <iostream>
#include <cmath>
#include "Matrix4x4.h"

using namespace std;


Matrix4x4::Matrix4x4()
{
    setIdentity();
}

void Matrix4x4::setIdentity()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
            {
                transform[i][j] = 1.0;
            }
            else
            {
                transform[i][j] = 0.0;
            }
        }
    }
}

double Matrix4x4::getValue(
    int row,
    int column) const
{
    return transform[row][column];
}

void Matrix4x4::print() const
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cout << transform[i][j] << " ";
        }

        cout << endl;
    }
}

void Matrix4x4::setTranslation(
    double x,
    double y,
    double z)
{
    transform[0][3] = x;

    transform[1][3] = y;

    transform[2][3] = z;
}

void Matrix4x4::setRotationZ(
    double angleDegrees)
{
    const double PI = 3.14159265358979323846;

    double angleRadians =
        angleDegrees * PI / 180.0;

    transform[0][0] = cos(angleRadians);
    transform[0][1] = -sin(angleRadians);
    transform[0][2] = 0.0;

    transform[1][0] = sin(angleRadians);
    transform[1][1] = cos(angleRadians);
    transform[1][2] = 0.0;

    transform[2][0] = 0.0;
    transform[2][1] = 0.0;
    transform[2][2] = 1.0;
}