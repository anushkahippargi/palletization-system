#include <iostream>
#include <sstream>
#include <string>

#include "Box.h"
#include "Pallet.h"
#include "Placement.h"
#include "PalletizationResult.h"
#include "PalletStatistics.h"
#include "RowAlgorithm.h"
#include "Statistics.h"

using namespace std;


/*
 * ============================================================
 * JSON OUTPUT
 * ============================================================
 *
 * Converts the actual PalletizationResult produced by the
 * RowAlgorithm into JSON.
 *
 * This is what the React 3D visualization will eventually use.
 */
void printJson(
    const PalletizationResult& result,
    const Box& box,
    const Pallet& pallet)
{
    const Statistics& statistics =
        result.getStatistics();

    const vector<Placement>& placements =
        result.getPlacements();


    cout << "{";


    /*
     * --------------------------------------------------------
     * BOX INFORMATION
     * --------------------------------------------------------
     */

    cout << "\"box\":{";

    cout << "\"length\":"
         << box.getLength()
         << ",";

    cout << "\"width\":"
         << box.getWidth()
         << ",";

    cout << "\"height\":"
         << box.getHeight();

    cout << "},";


    /*
     * --------------------------------------------------------
     * PALLET INFORMATION
     * --------------------------------------------------------
     */

    cout << "\"pallet\":{";

    cout << "\"length\":"
         << pallet.getLength()
         << ",";

    cout << "\"width\":"
         << pallet.getWidth()
         << ",";

    cout << "\"height\":"
         << pallet.getHeight();

    cout << "},";


    /*
     * --------------------------------------------------------
     * STATISTICS
     * --------------------------------------------------------
     */

    cout << "\"statistics\":{";

    cout << "\"totalBoxes\":"
         << statistics.getTotalBoxes()
         << ",";

    cout << "\"fullPallets\":"
         << statistics.getFullPallets();


    /*
     * Last pallet statistics are only included
     * when the last pallet is partially filled.
     */

    if (statistics.hasLastPallet())
    {
        const PalletStatistics&
            lastPallet =
                statistics.getLastPalletStatistics();

        cout << ",\"lastPallet\":{";

        cout << "\"palletId\":"
             << lastPallet.getPalletId()
             << ",";

        cout << "\"usedVolume\":"
             << lastPallet.getUsedVolume()
             << ",";

        cout << "\"totalVolume\":"
             << lastPallet.getTotalVolume()
             << ",";

        cout << "\"utilization\":"
             << lastPallet.getUtilization();

        cout << "}";
    }
    else
    {
        cout << ",\"lastPallet\":null";
    }

    cout << "},";


    /*
     * --------------------------------------------------------
     * PLACEMENTS
     * --------------------------------------------------------
     *
     * Every placement comes directly from the RO algorithm.
     */

    cout << "\"placements\":[";


    for (size_t i = 0;
         i < placements.size();
         i++)
    {
        const Placement& placement =
            placements[i];

        const Matrix4x4& pose =
            placement.getPose();


        if (i > 0)
        {
            cout << ",";
        }


        cout << "{";


        /*
         * Box ID
         */

        cout << "\"boxId\":"
             << placement.getBoxId()
             << ",";


        /*
         * Pallet ID
         */

        cout << "\"palletId\":"
             << placement.getPalletId()
             << ",";


        /*
         * Position
         *
         * Translation is stored in the final
         * column of the 4x4 pose matrix.
         */

        cout << "\"position\":[";

        cout << pose.get(0, 3)
             << ",";

        cout << pose.get(1, 3)
             << ",";

        cout << pose.get(2, 3);

        cout << "],";


        /*
         * Complete 4x4 pose matrix.
         */

        cout << "\"pose\":[";


        for (int row = 0;
             row < 4;
             row++)
        {
            if (row > 0)
            {
                cout << ",";
            }

            cout << "[";


            for (int column = 0;
                 column < 4;
                 column++)
            {
                if (column > 0)
                {
                    cout << ",";
                }

                cout << pose.get(
                    row,
                    column);
            }


            cout << "]";
        }


        cout << "]";

        cout << "}";
    }


    cout << "]";

    cout << "}" << endl;
}


/*
 * ============================================================
 * MAIN
 * ============================================================
 */

int main(int argc, char* argv[])
{
    /*
     * ========================================================
     * CUSTOM JSON MODE
     * ========================================================
     *
     * Usage:
     *
     * ./palletization --json \
     *     numberOfBoxes \
     *     boxLength \
     *     boxWidth \
     *     boxHeight \
     *     palletLength \
     *     palletWidth \
     *     palletHeight
     *
     * Example:
     *
     * ./palletization --json 45 300 200 150 1200 1000 300
     *
     * This executes the actual RO algorithm using the
     * supplied dimensions.
     */

    if (argc == 9 &&
        string(argv[1]) == "--json")
    {
        try
        {
            /*
             * Read custom input.
             */

            int quantity =
                stoi(argv[2]);

            double boxLength =
                stod(argv[3]);

            double boxWidth =
                stod(argv[4]);

            double boxHeight =
                stod(argv[5]);

            double palletLength =
                stod(argv[6]);

            double palletWidth =
                stod(argv[7]);

            double palletHeight =
                stod(argv[8]);


            /*
             * Basic validation.
             */

            if (quantity <= 0 ||
                boxLength <= 0 ||
                boxWidth <= 0 ||
                boxHeight <= 0 ||
                palletLength <= 0 ||
                palletWidth <= 0 ||
                palletHeight <= 0)
            {
                cerr
                    << "Error: All dimensions and "
                    << "number of boxes must be greater than 0."
                    << endl;

                return 1;
            }


            /*
             * Create Box using custom dimensions.
             *
             * Weight remains the existing test value
             * because the current frontend does not
             * provide weight.
             */

            Box box(
                boxLength,
                boxWidth,
                boxHeight,
                5.5);


            /*
             * Create Pallet using custom dimensions.
             *
             * Pallet weight remains the existing
             * value because the current frontend
             * does not provide it.
             */

            Pallet pallet(
                palletLength,
                palletWidth,
                palletHeight,
                1000);


            /*
             * Run the actual RO algorithm.
             */

            RowAlgorithm algorithm;


            /*
             * RowAlgorithm currently prints diagnostic
             * information to cout.
             *
             * JSON mode must return ONLY JSON.
             *
             * Therefore temporarily redirect cout
             * while the algorithm is running.
             */

            ostringstream debugOutput;

            streambuf* originalBuffer =
                cout.rdbuf(
                    debugOutput.rdbuf());


            PalletizationResult result =
                algorithm.generatePattern(
                    pallet,
                    box,
                    quantity);


            /*
             * Restore cout.
             */

            cout.rdbuf(
                originalBuffer);


            /*
             * Output the actual algorithm result
             * as JSON.
             */

            printJson(
                result,
                box,
                pallet);


            return 0;
        }
        catch (const exception& error)
        {
            cerr
                << "Error: Invalid input."
                << endl;

            cerr
                << error.what()
                << endl;

            return 1;
        }
    }


    /*
     * ========================================================
     * DEFAULT TEST MODE
     * ========================================================
     *
     * Running:
     *
     * ./palletization
     *
     * still performs the existing 45-box test.
     */

    Box box(
        300,
        200,
        150,
        5.5);


    Pallet pallet(
        1200,
        1000,
        300,
        1000);


    RowAlgorithm algorithm;


    /*
     * Test quantity:
     *
     * Pallet capacity = 40
     * Requested boxes = 45
     *
     * Expected:
     *
     * Pallet 1 -> 40 boxes -> Full
     * Pallet 2 -> 5 boxes  -> Partial
     */

    PalletizationResult result =
        algorithm.generatePattern(
            pallet,
            box,
            45);


    /*
     * Display all placements.
     */

    for (const Placement& placement :
         result.getPlacements())
    {
        placement.print();

        cout << endl;
    }


    /*
     * Display statistics.
     */

    cout << "\nStatistics"
         << endl;


    const Statistics& statistics =
        result.getStatistics();


    cout << "Total Boxes : "
         << statistics.getTotalBoxes()
         << endl;


    cout << "Full Pallets : "
         << statistics.getFullPallets()
         << endl;


    /*
     * Only display last pallet statistics
     * when the final pallet is partially filled.
     */

    if (statistics.hasLastPallet())
    {
        const PalletStatistics&
            lastPallet =
                statistics.getLastPalletStatistics();


        cout << "\nLast Pallet"
             << endl;


        cout << "Pallet ID : "
             << lastPallet.getPalletId()
             << endl;


        cout << "Used Volume : "
             << lastPallet.getUsedVolume()
             << endl;


        cout << "Total Volume : "
             << lastPallet.getTotalVolume()
             << endl;


        cout << "Utilization : "
             << lastPallet.getUtilization()
             << "%"
             << endl;
    }


    return 0;
}