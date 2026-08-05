#include "RowAlgorithm.h"

using namespace std;

PalletizationResult RowAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;
    double boxVolume =
    box.getLength() *
    box.getWidth() *
    box.getHeight();

double palletVolume =
    pallet.getLength() *
    pallet.getWidth() *
    pallet.getHeight();

    double x = 0;
    double y = 0;
    double z = 0;
    int boxesPlaced = 0;

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

        result.getPlacements().push_back(
            placement);
            boxesPlaced++;

        // Move to the next position
        x += box.getLength();
    }
Statistics& statistics =
    result.getStatistics();

statistics.setTotalBoxes(
    boxesPlaced);

if (boxesPlaced > 0)
{
    statistics.setUsedPallets(
        1);
}
else
{
    statistics.setUsedPallets(
        0);
}

PalletStatistics palletStatistics(
    1,
    boxesPlaced * boxVolume,
    palletVolume);

statistics.addPalletStatistics(
    palletStatistics);
    return result;
}