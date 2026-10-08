#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "BricksAlgorithm.h"
#include "Matrix4x4.h"
#include "PalletStatistics.h"

using namespace std;

namespace
{
    constexpr double EPS = 1e-6;

    struct Rect
    {
        double x = 0.0;
        double y = 0.0;
        double fl = 0.0;
        double fw = 0.0;
        int rot = 0;
    };

    using Layer = vector<Rect>;

    struct Dims
    {
        double L = 0.0;
        double W = 0.0;
        double l = 0.0;
        double w = 0.0;
    };


    // ============================================================
    // RECTANGLE CREATION
    // ============================================================

    Rect makeRect(double x, double y, int rot, const Dims& d)
    {
        Rect r;

        r.x = x;
        r.y = y;
        r.rot = rot;

        if (rot == 0)
        {
            r.fl = d.l;
            r.fw = d.w;
        }
        else
        {
            r.fl = d.w;
            r.fw = d.l;
        }

        return r;
    }


    // ============================================================
    // BASIC GEOMETRY
    // ============================================================

    bool insidePallet(const Rect& r, const Dims& d)
    {
        return
            r.x >= -EPS &&
            r.y >= -EPS &&
            r.x + r.fl <= d.L + EPS &&
            r.y + r.fw <= d.W + EPS;
    }

    double overlapLength(
        double a0,
        double a1,
        double b0,
        double b1)
    {
        return min(a1, b1) - max(a0, b0);
    }

    bool intersects(const Rect& a, const Rect& b)
    {
        return
            overlapLength(
                a.x,
                a.x + a.fl,
                b.x,
                b.x + b.fl) > EPS
            &&
            overlapLength(
                a.y,
                a.y + a.fw,
                b.y,
                b.y + b.fw) > EPS;
    }

    bool collides(const Layer& layer, const Rect& r)
    {
        for (const Rect& other : layer)
        {
            if (intersects(other, r))
                return true;
        }

        return false;
    }


    // ============================================================
    // SUPPORT CHECK
    // ============================================================

    double supportedArea(
        const Rect& r,
        const Layer& lower)
    {
        double area = 0.0;

        for (const Rect& b : lower)
        {
            const double ox =
                overlapLength(
                    r.x,
                    r.x + r.fl,
                    b.x,
                    b.x + b.fl);

            const double oy =
                overlapLength(
                    r.y,
                    r.y + r.fw,
                    b.y,
                    b.y + b.fw);

            if (ox > EPS && oy > EPS)
            {
                area += ox * oy;
            }
        }

        return area;
    }

    bool fullySupported(
        const Rect& r,
        const Layer& lower)
    {
        const double required =
            r.fl * r.fw;

        const double actual =
            supportedArea(r, lower);

        return actual >= required - EPS;
    }

    bool layerFullySupported(
        const Layer& upper,
        const Layer& lower)
    {
        for (const Rect& r : upper)
        {
            if (!fullySupported(r, lower))
                return false;
        }

        return true;
    }


    // ============================================================
    // LAYER VALIDATION
    // ============================================================

    bool validLayer(
        const Layer& layer,
        const Dims& d)
    {
        if (layer.empty())
            return false;

        for (size_t i = 0; i < layer.size(); ++i)
        {
            if (!insidePallet(layer[i], d))
                return false;

            for (size_t j = i + 1; j < layer.size(); ++j)
            {
                if (intersects(layer[i], layer[j]))
                    return false;
            }
        }

        return true;
    }


    // ============================================================
    // LAYER SORTING
    // ============================================================

    void sortLayer(Layer& layer)
    {
        sort(
            layer.begin(),
            layer.end(),
            [](const Rect& a, const Rect& b)
            {
                if (fabs(a.y - b.y) > EPS)
                    return a.y < b.y;

                if (fabs(a.x - b.x) > EPS)
                    return a.x < b.x;

                return a.rot < b.rot;
            });
    }


    // ============================================================
    // LAYER 1
    //
    // Brick / nested-frame pattern.
    //
    // For the standard project case:
    //
    // Box    = 300 x 200
    // Pallet = 1200 x 1000
    //
    // This creates exactly 20 boxes:
    //
    //  ┌──┬──┬──┬──┐
    //  │  │  │  │  │
    //  ├──┤  ┌──┬──┤
    //  │  │  │  │  │
    //  │  │  ├──┤  │
    //  │  │  ├──┤  │
    //  ├──┤  │  ├──┤
    //  │  │  │  │  │
    //  ├──┴──┴──┴──┤
    //  └──┴──┴──┴──┘
    //
    // ============================================================

    Layer makeStandardBrickLayer(const Dims& d)
    {
        Layer layer;

        /*
         * This pattern is specifically based on the geometry
         * that has been working for the project:
         *
         * pallet = 1200 x 1000
         * box    = 300 x 200
         *
         * The coordinates are generated proportionally from
         * the box dimensions so the same pattern works when
         * the dimensions preserve the 4 x 5 relationship.
         */

        const double l = d.l;
        const double w = d.w;

        // --------------------------------------------------------
        // TOP ROW - 4 horizontal boxes
        // --------------------------------------------------------

        for (int i = 0; i < 4; ++i)
        {
            layer.push_back(
                makeRect(
                    i * l,
                    0.0,
                    0,
                    d));
        }


        // --------------------------------------------------------
        // LEFT SIDE - upper and lower vertical boxes
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                0.0,
                w,
                90,
                d));

        layer.push_back(
            makeRect(
                0.0,
                2.5 * w,
                90,
                d));


        // --------------------------------------------------------
        // RIGHT SIDE - upper and lower vertical boxes
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                d.L - w,
                w,
                90,
                d));

        layer.push_back(
            makeRect(
                d.L - w,
                2.5 * w,
                90,
                d));


        // --------------------------------------------------------
        // INNER LEFT VERTICAL
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                w,
                w,
                90,
                d));


        // --------------------------------------------------------
        // INNER UPPER HORIZONTAL PAIR
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                2.0 * w,
                w,
                0,
                d));

        layer.push_back(
            makeRect(
                2.0 * w + l,
                w,
                0,
                d));


        // --------------------------------------------------------
        // INNER MIDDLE HORIZONTAL PAIR
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                2.0 * w,
                2.0 * w,
                0,
                d));

        layer.push_back(
            makeRect(
                2.0 * w + l,
                2.0 * w,
                0,
                d));


        // --------------------------------------------------------
        // INNER LOWER LEFT VERTICAL
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                w,
                2.5 * w,
                90,
                d));


        // --------------------------------------------------------
        // INNER LOWER HORIZONTAL PAIR
        // --------------------------------------------------------

        layer.push_back(
            makeRect(
                2.0 * w,
                3.0 * w,
                0,
                d));

        layer.push_back(
            makeRect(
                2.0 * w + l,
                3.0 * w,
                0,
                d));


        // --------------------------------------------------------
        // BOTTOM ROW - 4 horizontal boxes
        // --------------------------------------------------------

        for (int i = 0; i < 4; ++i)
        {
            layer.push_back(
                makeRect(
                    i * l,
                    d.W - w,
                    0,
                    d));
        }

        sortLayer(layer);

        return layer;
    }


    // ============================================================
    // EXACT STANDARD PATTERN
    //
    // The standard project dimensions are:
    //
    // pallet = 1200 x 1000
    // box    = 300 x 200
    //
    // If those proportions are present, use the real brick
    // pattern above.
    // ============================================================

    bool isStandardGeometry(const Dims& d)
    {
        return
            fabs(d.L - 4.0 * d.l) <= EPS &&
            fabs(d.W - 5.0 * d.w) <= EPS &&
            fabs(d.l - 1.5 * d.w) <= EPS;
    }


    // ============================================================
    // GENERIC FALLBACK
    //
    // Used only when the dimensions are not the standard
    // 4 x 5 brick geometry.
    // ============================================================

    Layer makeGenericLayer(const Dims& d)
    {
        Layer best;

        Layer horizontal;

        const int cols =
            static_cast<int>(
                floor(d.L / d.l + EPS));

        const int rows =
            static_cast<int>(
                floor(d.W / d.w + EPS));

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                horizontal.push_back(
                    makeRect(
                        col * d.l,
                        row * d.w,
                        0,
                        d));
            }
        }

        Layer vertical;

        const int vCols =
            static_cast<int>(
                floor(d.L / d.w + EPS));

        const int vRows =
            static_cast<int>(
                floor(d.W / d.l + EPS));

        for (int row = 0; row < vRows; ++row)
        {
            for (int col = 0; col < vCols; ++col)
            {
                vertical.push_back(
                    makeRect(
                        col * d.w,
                        row * d.l,
                        90,
                        d));
            }
        }

        if (validLayer(horizontal, d))
            best = horizontal;

        if (validLayer(vertical, d) &&
            vertical.size() > best.size())
        {
            best = vertical;
        }

        sortLayer(best);

        return best;
    }


    // ============================================================
    // MIRROR LAYER
    //
    // IMPORTANT:
    //
    // Layer 2 is NOT generated as another independent grid.
    //
    // It is the exact horizontal mirror of Layer 1.
    //
    // Since Layer 1 completely covers the pallet, its mirror
    // also completely covers the pallet.
    //
    // This guarantees:
    //     - same box count
    //     - no gaps
    //     - no overlap
    //     - full support
    //     - different brick joints
    // ============================================================

    Layer mirrorX(
        const Layer& source,
        const Dims& d)
    {
        Layer mirrored;

        for (const Rect& r : source)
        {
            Rect m = r;

            m.x =
                d.L - r.x - r.fl;

            mirrored.push_back(m);
        }

        sortLayer(mirrored);

        return mirrored;
    }


    // ============================================================
    // ADD PLACEMENT
    // ============================================================

    void addPlacement(
        PalletizationResult& result,
        int boxId,
        int palletId,
        const Rect& r,
        double z)
    {
        Matrix4x4 pose;

        pose.setTranslation(
            r.x,
            r.y,
            z);

        pose.setRotationZ(
            static_cast<double>(r.rot));

        result.getPlacements().emplace_back(
            boxId,
            palletId,
            pose);
    }


    // ============================================================
    // AREA / INTERLOCK INFORMATION
    // ============================================================

    double interlockScore(
        const Layer& upper,
        const Layer& lower)
    {
        if (upper.empty() || lower.empty())
            return 0.0;

        int bridging = 0;

        for (const Rect& u : upper)
        {
            int touched = 0;

            for (const Rect& l : lower)
            {
                const double ox =
                    overlapLength(
                        u.x,
                        u.x + u.fl,
                        l.x,
                        l.x + l.fl);

                const double oy =
                    overlapLength(
                        u.y,
                        u.y + u.fw,
                        l.y,
                        l.y + l.fw);

                if (ox > EPS && oy > EPS)
                    ++touched;
            }

            if (touched >= 2)
                ++bridging;
        }

        return
            static_cast<double>(bridging) /
            static_cast<double>(upper.size());
    }
}


// ================================================================
// MAIN BRICKS ALGORITHM
// ================================================================

PalletizationResult
BricksAlgorithm::generatePattern(
    const Pallet& pallet,
    const Box& box,
    int quantity)
{
    PalletizationResult result;

    if (quantity <= 0)
        return result;

    if (box.getLength() <= 0.0 ||
        box.getWidth() <= 0.0 ||
        box.getHeight() <= 0.0)
    {
        return result;
    }

    if (pallet.getLength() <= 0.0 ||
        pallet.getWidth() <= 0.0 ||
        pallet.getHeight() <= 0.0)
    {
        return result;
    }


    // ============================================================
    // DIMENSIONS
    // ============================================================

    Dims d;

    d.L = pallet.getLength();
    d.W = pallet.getWidth();
    d.l = box.getLength();
    d.w = box.getWidth();


    // ============================================================
    // NUMBER OF LAYERS
    // ============================================================

    const int layersPerPallet =
        static_cast<int>(
            floor(
                pallet.getHeight() /
                box.getHeight() +
                EPS));

    if (layersPerPallet <= 0)
        return result;


    // ============================================================
    // BUILD LAYER 1
    // ============================================================

    Layer layer1;

    if (isStandardGeometry(d))
    {
        layer1 =
            makeStandardBrickLayer(d);
    }
    else
    {
        layer1 =
            makeGenericLayer(d);
    }


    // ------------------------------------------------------------
    // Safety validation
    // ------------------------------------------------------------

    if (!validLayer(layer1, d))
    {
        return result;
    }


    // ============================================================
    // BUILD LAYER 2
    // ============================================================

    vector<Layer> layers;

    layers.push_back(layer1);


    if (layersPerPallet >= 2)
    {
        Layer layer2 =
            mirrorX(layer1, d);

        /*
         * The mirrored layer must itself be a valid complete
         * pallet layer.
         */
        if (validLayer(layer2, d) &&
            layer2.size() == layer1.size() &&
            layerFullySupported(layer2, layer1))
        {
            layers.push_back(layer2);
        }
        else
        {
            /*
             * Extremely conservative fallback:
             * repeat Layer 1 only if it is fully supported.
             */
            if (layerFullySupported(layer1, layer1))
            {
                layers.push_back(layer1);
            }
        }
    }


    // ============================================================
    // ADD MORE LAYERS
    //
    // Alternate the two brick courses:
    //
    // Layer 1 = original
    // Layer 2 = mirrored
    // Layer 3 = original
    // Layer 4 = mirrored
    //
    // Every layer is checked against the actual layer directly
    // below it.
    // ============================================================

    while (
        static_cast<int>(layers.size()) <
        layersPerPallet)
    {
        const Layer& lower =
            layers.back();

        const bool useOriginal =
            (layers.size() % 2 == 0);

        Layer candidate;

        if (useOriginal)
        {
            candidate = layer1;
        }
        else
        {
            candidate = mirrorX(layer1, d);
        }

        /*
         * Only accept the course when every single box is
         * completely supported by the previous layer.
         */
        if (!validLayer(candidate, d) ||
            candidate.size() != layer1.size() ||
            !layerFullySupported(candidate, lower))
        {
            break;
        }

        layers.push_back(candidate);
    }


    // ============================================================
    // DEBUG INFORMATION
    // ============================================================

    cout << endl;
    cout << "========================================" << endl;
    cout << "BRICKS ALGORITHM" << endl;
    cout << "========================================" << endl;

    cout << "Box: "
         << d.l << " x "
         << d.w << " x "
         << box.getHeight()
         << endl;

    cout << "Pallet: "
         << d.L << " x "
         << d.W << " x "
         << pallet.getHeight()
         << endl;

    cout << "Layers per pallet: "
         << layers.size()
         << " / "
         << layersPerPallet
         << endl;

    for (size_t i = 0;
         i < layers.size();
         ++i)
    {
        cout << "Layer "
             << (i + 1)
             << ": "
             << layers[i].size()
             << " boxes";

        if (i == 0)
        {
            cout << " (original brick pattern)";
        }
        else if (i % 2 == 1)
        {
            const double score =
                interlockScore(
                    layers[i],
                    layers[i - 1]);

            cout << " (mirrored brick pattern, interlock "
                 << static_cast<int>(
                        round(score * 100.0))
                 << "%)";
        }
        else
        {
            cout << " (original brick pattern)";
        }

        cout << endl;
    }


    // ============================================================
    // PALLET CAPACITY
    // ============================================================

    int palletCapacity = 0;

    for (const Layer& layer : layers)
    {
        palletCapacity +=
            static_cast<int>(
                layer.size());
    }

    if (palletCapacity <= 0)
        return result;


    cout << "Capacity per pallet: "
         << palletCapacity
         << endl;

    cout << "========================================"
         << endl;


    // ============================================================
    // VOLUME STATISTICS
    // ============================================================

    const double boxVolume =
        box.getLength() *
        box.getWidth() *
        box.getHeight();

    const double palletVolume =
        pallet.getLength() *
        pallet.getWidth() *
        pallet.getHeight();


    // ============================================================
    // GENERATE PALLETS
    // ============================================================

    int boxesPlaced = 0;
    int fullPallets = 0;
    int palletId = 1;

    while (boxesPlaced < quantity)
    {
        const int beforePallet =
            boxesPlaced;

        for (size_t layerIndex = 0;
             layerIndex < layers.size() &&
             boxesPlaced < quantity;
             ++layerIndex)
        {
            const double z =
                static_cast<double>(
                    layerIndex) *
                box.getHeight();

            for (const Rect& position :
                 layers[layerIndex])
            {
                if (boxesPlaced >= quantity)
                    break;

                addPlacement(
                    result,
                    boxesPlaced + 1,
                    palletId,
                    position,
                    z);

                ++boxesPlaced;
            }
        }

        const int boxesOnPallet =
            boxesPlaced -
            beforePallet;


        // --------------------------------------------------------
        // FULL PALLET
        // --------------------------------------------------------

        if (boxesOnPallet ==
            palletCapacity)
        {
            ++fullPallets;
        }

        // --------------------------------------------------------
        // PARTIAL PALLET
        // --------------------------------------------------------

        else if (boxesOnPallet > 0)
        {
            result.getStatistics()
                .setLastPalletStatistics(
                    PalletStatistics(
                        palletId,
                        boxesOnPallet *
                            boxVolume,
                        palletVolume));
        }


        ++palletId;


        if (boxesOnPallet <= 0)
            break;
    }


    // ============================================================
    // FINAL STATISTICS
    // ============================================================

    result.getStatistics()
        .setTotalBoxes(
            boxesPlaced);

    result.getStatistics()
        .setFullPallets(
            fullPallets);

    return result;
}