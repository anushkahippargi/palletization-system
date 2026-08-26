import http from "http";
import { spawnSync } from "child_process";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const PORT = 3001;

/*
 * Path to the compiled C++ palletization executable.
 */
const executablePath = path.join(
  __dirname,
  "..",
  "build",
  "palletization"
);

/*
 * Send JSON response with CORS headers.
 */
function sendJSON(res, statusCode, data) {
  const body = JSON.stringify(data);

  res.writeHead(statusCode, {
    "Content-Type": "application/json",
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Methods":
      "GET, POST, OPTIONS",
    "Access-Control-Allow-Headers":
      "Content-Type",
  });

  res.end(body);
}

/*
 * Read and parse the JSON request body.
 */
function readBody(req) {
  return new Promise((resolve, reject) => {
    let body = "";

    req.on("data", (chunk) => {
      body += chunk;
    });

    req.on("end", () => {
      try {
        resolve(JSON.parse(body));
      } catch (error) {
        reject(
          new Error("Invalid JSON request body")
        );
      }
    });

    req.on("error", reject);
  });
}

const server = http.createServer(
  async (req, res) => {
    /*
     * =====================================
     * HANDLE CORS PREFLIGHT REQUEST
     * =====================================
     *
     * The React frontend sends an OPTIONS
     * request before the POST request.
     */

    if (req.method === "OPTIONS") {
      res.writeHead(204, {
        "Access-Control-Allow-Origin": "*",
        "Access-Control-Allow-Methods":
          "GET, POST, OPTIONS",
        "Access-Control-Allow-Headers":
          "Content-Type",
      });

      return res.end();
    }

    /*
     * =====================================
     * HEALTH CHECK
     * =====================================
     */

    if (
      req.method === "GET" &&
      req.url === "/api/health"
    ) {
      return sendJSON(res, 200, {
        status: "ok",
      });
    }

    /*
     * =====================================
     * MAIN PALLETIZATION ENDPOINT
     * =====================================
     */

    if (
      req.method === "POST" &&
      req.url === "/api/palletize"
    ) {
      try {
        /*
         * Read request data from React.
         */
        const data = await readBody(req);

        /*
         * =====================================
         * VALIDATE INPUT
         * =====================================
         */

        const boxCount =
          Number(data.boxCount);

        const boxLength =
          Number(data.box?.length);

        const boxWidth =
          Number(data.box?.width);

        const boxHeight =
          Number(data.box?.height);

        const palletLength =
          Number(data.pallet?.length);

        const palletWidth =
          Number(data.pallet?.width);

        const palletHeight =
          Number(data.pallet?.height);

        const values = [
          boxCount,
          boxLength,
          boxWidth,
          boxHeight,
          palletLength,
          palletWidth,
          palletHeight,
        ];

        /*
         * All values must be valid positive numbers.
         */

        if (
          values.some(
            (value) =>
              !Number.isFinite(value) ||
              value <= 0
          )
        ) {
          return sendJSON(res, 400, {
            error:
              "All input values must be positive numbers.",
          });
        }

        /*
         * =====================================
         * RUN C++ PALLETIZATION ALGORITHM
         * =====================================
         *
         * Command generated:
         *
         * palletization --json
         * boxCount
         * boxLength
         * boxWidth
         * boxHeight
         * palletLength
         * palletWidth
         * palletHeight
         */

        const result = spawnSync(
          executablePath,
          [
            "--json",
            String(boxCount),
            String(boxLength),
            String(boxWidth),
            String(boxHeight),
            String(palletLength),
            String(palletWidth),
            String(palletHeight),
          ],
          {
            encoding: "utf8",
            maxBuffer:
              50 * 1024 * 1024,
          }
        );

        /*
         * =====================================
         * HANDLE EXECUTION ERROR
         * =====================================
         */

        if (result.error) {
          console.error(
            "C++ execution error:",
            result.error
          );

          return sendJSON(res, 500, {
            error:
              "Could not execute the palletization algorithm.",
            details:
              result.error.message,
          });
        }

        /*
         * =====================================
         * HANDLE C++ PROGRAM FAILURE
         * =====================================
         */

        if (result.status !== 0) {
          console.error(
            "C++ stderr:",
            result.stderr
          );

          return sendJSON(res, 500, {
            error:
              "Palletization algorithm failed.",
            details:
              result.stderr ||
              "Unknown C++ error",
          });
        }

        /*
         * =====================================
         * PARSE C++ JSON OUTPUT
         * =====================================
         */

        let algorithmResult;

        try {
          algorithmResult =
            JSON.parse(result.stdout);
        } catch (error) {
          console.error(
            "Invalid JSON returned from C++:"
          );

          console.error(
            result.stdout
          );

          return sendJSON(res, 500, {
            error:
              "C++ program did not return valid JSON.",
            details:
              result.stdout,
          });
        }

        /*
         * =====================================
         * RETURN REAL ALGORITHM OUTPUT
         * TO REACT
         * =====================================
         */

        return sendJSON(
          res,
          200,
          algorithmResult
        );
      } catch (error) {
        console.error(
          "Server error:",
          error
        );

        return sendJSON(res, 400, {
          error:
            error.message ||
            "Something went wrong while processing the request.",
        });
      }
    }

    /*
     * =====================================
     * UNKNOWN ROUTE
     * =====================================
     */

    return sendJSON(res, 404, {
      error: "Route not found",
    });
  }
);

/*
 * =====================================
 * START BACKEND SERVER
 * =====================================
 */

server.listen(PORT, () => {
  console.log(
    `Palletization backend running at http://localhost:${PORT}`
  );

  console.log(
    `Using C++ executable: ${executablePath}`
  );
});