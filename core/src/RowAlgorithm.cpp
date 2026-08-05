#include <vector>

#include "RowAlgorithm.h"

using namespace std;

vector<Placement> RowAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    vector<Placement> placements;

    double x = 0;
    double y = 0;
    double z = 0;

    for (int i = 1; i <= quantity; i++)
    {
        // Check if the box fits in the current row
        if (x + box.getLength() > pallet.getLength())
        {
            x = 0;
            y += box.getWidth();
        }

        // Check if there is space on the pallet
        if (y + box.getWidth() > pallet.getWidth())
        {
            break;
        }

        // Create the pose (position + orientation)
        Matrix4x4 pose;

        pose.setTranslation(
            x,
            y,
            z);

        pose.setRotationZ(0);

        // Create the placement
        Placement placement(
            i,
            pose);

        placements.push_back(
            placement);

        // Move to the next position
        x += box.getLength();
    }

    return placements;
}