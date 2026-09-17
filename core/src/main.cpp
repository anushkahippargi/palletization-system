#include <iostream>
#include <sstream>
#include <string>

#include "Box.h"
#include "Pallet.h"
#include "Placement.h"
#include "PalletizationResult.h"
#include "PalletStatistics.h"
#include "RowAlgorithm.h"
#include "RowsColsAlgorithm.h"
#include "Statistics.h"
#include "WheelAlgorithm.h"
#include "WheelInnerFillAlgorithm.h"
#include "BricksAlgorithm.h"

using namespace std;


/*
 * ============================================================
 * JSON OUTPUT
 * ============================================================
 *
 * Converts the PalletizationResult produced by the
 * selected algorithm into JSON.
 *
 * Supported algorithms:
 *
 *     rows
 *     rowscols
 *
 * The React 3D visualization can use the same JSON structure
 * regardless of which algorithm produced the result.
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
     * Every placement comes from whichever algorithm
     * was selected.
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
     * Existing usage:
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
     *
     * New usage:
     *
     * ./palletization --json rowscols \
     *     numberOfBoxes \
     *     boxLength \
     *     boxWidth \
     *     boxHeight \
     *     palletLength \
     *     palletWidth \
     *     palletHeight
     *
     *
     * Examples:
     *
     * Existing Rows algorithm:
     *
     * ./palletization --json \
     *     45 300 200 150 1200 1000 300
     *
     *
     * New Rows + Columns algorithm:
     *
     * ./palletization --json rowscols \
     *     45 300 200 150 1200 1000 300
     */

    if ((argc == 9 || argc == 10) &&
        string(argv[1]) == "--json")
    {
        try
        {
            /*
             * ------------------------------------------------
             * Determine which algorithm to use.
             * ------------------------------------------------
             *
             * If no algorithm is supplied,
             * default to the existing Rows algorithm.
             */

            string algorithmName = "rows";

            int inputIndex = 2;


            /*
             * If argc == 10, the third argument
             * is the algorithm name.
             *
             * Example:
             *
             * argv[2] = "rowscols"
             * argv[3] = quantity
             */

            if (argc == 10)
            {
                algorithmName =
                    argv[2];

                inputIndex = 3;
            }


            /*
             * ------------------------------------------------
             * Read custom input.
             * ------------------------------------------------
             */

            int quantity =
                stoi(argv[inputIndex]);

            double boxLength =
                stod(argv[inputIndex + 1]);

            double boxWidth =
                stod(argv[inputIndex + 2]);

            double boxHeight =
                stod(argv[inputIndex + 3]);

            double palletLength =
                stod(argv[inputIndex + 4]);

            double palletWidth =
                stod(argv[inputIndex + 5]);

            double palletHeight =
                stod(argv[inputIndex + 6]);


            /*
             * ------------------------------------------------
             * Basic validation.
             * ------------------------------------------------
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
             * ------------------------------------------------
             * Create Box.
             * ------------------------------------------------
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
             * ------------------------------------------------
             * Create Pallet.
             * ------------------------------------------------
             *
             * Pallet weight remains the existing test
             * value because the current frontend does
             * not provide it.
             */

            Pallet pallet(
                palletLength,
                palletWidth,
                palletHeight,
                1000);


            /*
             * ------------------------------------------------
             * Temporarily redirect cout.
             * ------------------------------------------------
             *
             * Some algorithms print diagnostic information.
             *
             * JSON mode must output ONLY JSON.
             *
             * Therefore we temporarily redirect cout
             * while the algorithm is running.
             */

            ostringstream debugOutput;

            streambuf* originalBuffer =
                cout.rdbuf(
                    debugOutput.rdbuf());


            /*
             * ------------------------------------------------
             * Run selected algorithm.
             * ------------------------------------------------
             */

            PalletizationResult result;


            /*
             * Existing Rows algorithm
             */

if (algorithmName == "rows")
{
    RowAlgorithm algorithm;

    result = algorithm.generatePattern(
        pallet,
        box,
        quantity
    );
}
else if (algorithmName == "rowscols")
{
    RowsColsAlgorithm algorithm;

    result = algorithm.generatePattern(
        pallet,
        box,
        quantity
    );
}
else if (algorithmName == "wheel")
{
    WheelAlgorithm algorithm;

    result = algorithm.generatePattern(
        pallet,
        box,
        quantity
    );
}
else if (algorithmName == "wheelinnerfill")
{
    WheelInnerFillAlgorithm algorithm;

    result = algorithm.generatePattern(
        pallet,
        box,
        quantity
    );
}
else if (algorithmName == "bricks")
{
    BricksAlgorithm algorithm;
    result = algorithm.generatePattern(
        pallet,
        box,
        quantity);
}
else
{
    std::cerr
        << "Unknown algorithm: "
        << algorithmName
        << std::endl;

    std::cerr
        << "Available algorithms: "
        << "rows, "
        << "rowscols, "
        << "wheel, "
        << "wheelinnerfill"
        << std::endl;

    return 1;
}

            /*
             * ------------------------------------------------
             * Restore cout.
             * ------------------------------------------------
             */

            cout.rdbuf(
                originalBuffer);


            /*
             * ------------------------------------------------
             * Output actual algorithm result as JSON.
             * ------------------------------------------------
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
     * still performs the existing 45-box test
     * using the original RowAlgorithm.
     *
     * This means we have NOT broken the old behavior.
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


    /*
     * Keep the default test using the existing
     * RowAlgorithm.
     */

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
     * --------------------------------------------------------
     * Display all placements.
     * --------------------------------------------------------
     */

    for (const Placement& placement :
         result.getPlacements())
    {
        placement.print();

        cout << endl;
    }


    /*
     * --------------------------------------------------------
     * Display statistics.
     * --------------------------------------------------------
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