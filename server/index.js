import express from "express";
import cors from "cors";
import { execFile } from "child_process";
import path from "path";
import { fileURLToPath } from "url";

const app = express();

app.use(cors());
app.use(express.json());


// ============================================================
// ES Module __dirname setup
// ============================================================

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);


// ============================================================
// Palletization API
// ============================================================

app.post("/api/palletize", (req, res) => {

    const {
        algorithm = "rows",
        boxCount,
        boxLength,
        boxWidth,
        boxHeight,
        palletLength,
        palletWidth,
        palletHeight
    } = req.body;


    // ----------------------------------------------------------
    // Validate algorithm
    // ----------------------------------------------------------

    const allowedAlgorithms = [
        "rows",
        "rowscols"
    ];

    if (!allowedAlgorithms.includes(algorithm)) {
        return res.status(400).json({
            error: "Invalid algorithm. Use 'rows' or 'rowscols'."
        });
    }


    // ----------------------------------------------------------
    // Validate input values
    // ----------------------------------------------------------

    const values = [
        boxCount,
        boxLength,
        boxWidth,
        boxHeight,
        palletLength,
        palletWidth,
        palletHeight
    ];

    if (
        values.some(
            value =>
                value === undefined ||
                value === null ||
                Number(value) <= 0
        )
    ) {
        return res.status(400).json({
            error: "All box and pallet values must be greater than 0."
        });
    }


    // ----------------------------------------------------------
    // C++ executable
    // ----------------------------------------------------------

    const executablePath = path.join(
        __dirname,
        "..",
        "build",
        "palletization"
    );


    // ----------------------------------------------------------
    // Arguments sent to C++
    // ----------------------------------------------------------

    const args = [
        "--json",
        algorithm,
        String(boxCount),
        String(boxLength),
        String(boxWidth),
        String(boxHeight),
        String(palletLength),
        String(palletWidth),
        String(palletHeight)
    ];


    console.log(
        `Running algorithm: ${algorithm}`
    );

    console.log(
        `Arguments: ${args.join(" ")}`
    );


    // ----------------------------------------------------------
    // Run C++ executable
    // ----------------------------------------------------------

    execFile(
        executablePath,
        args,
        (error, stdout, stderr) => {

            if (error) {

                console.error(
                    "Palletization error:",
                    error
                );

                console.error(
                    "C++ stderr:",
                    stderr
                );

                return res.status(500).json({
                    error: "Palletization failed.",
                    details: stderr || error.message
                });
            }


            // --------------------------------------------------
            // Parse C++ JSON
            // --------------------------------------------------

            try {

                const result =
                    JSON.parse(stdout);

                return res.json(result);

            } catch (parseError) {

                console.error(
                    "Invalid JSON received from C++:"
                );

                console.error(stdout);

                return res.status(500).json({
                    error:
                        "Invalid response from palletization engine.",
                    details: parseError.message
                });
            }
        }
    );
});


// ============================================================
// Health check
// ============================================================

app.get("/api/health", (req, res) => {

    res.json({
        status: "ok",
        message: "Palletization backend is running."
    });

});


// ============================================================
// Start server
// ============================================================

const PORT = 3001;

app.listen(PORT, () => {

    console.log(
        `Palletization server running on port ${PORT}`
    );

});