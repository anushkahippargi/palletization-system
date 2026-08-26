import { useCallback, useState } from "react";
import "./App.css";
import PalletScene from "./components/PalletScene";

function App() {
  const [form, setForm] = useState({
    boxCount: 10,
    boxLength: 300,
    boxWidth: 200,
    boxHeight: 150,
    palletLength: 1200,
    palletWidth: 1000,
    palletHeight: 300,
  });

  const [page, setPage] = useState("home");

  const [result, setResult] = useState(null);

  const [loading, setLoading] = useState(false);

  const [error, setError] = useState("");

  const [animationKey, setAnimationKey] =
    useState(0);

  const [animationState, setAnimationState] =
    useState({
      placedCount: 0,
      playing: false,
    });

  const [animationCommand, setAnimationCommand] =
    useState({
      type: "none",
      id: 0,
    });

  const handleChange = (event) => {
    const { name, value } = event.target;

    setForm((previous) => ({
      ...previous,
      [name]: Number(value),
    }));
  };

  /*
   * Send the user's custom input
   * to the C++ backend.
   */

  const handleSubmit = async (event) => {
    event.preventDefault();

    setLoading(true);
    setError("");

    try {
      const response = await fetch(
        "http://localhost:3001/api/palletize",
        {
          method: "POST",

          headers: {
            "Content-Type": "application/json",
          },

          body: JSON.stringify({
            boxCount: form.boxCount,

            box: {
              length: form.boxLength,
              width: form.boxWidth,
              height: form.boxHeight,
            },

            pallet: {
              length: form.palletLength,
              width: form.palletWidth,
              height: form.palletHeight,
            },
          }),
        }
      );

      const data =
        await response.json();

      if (!response.ok) {
        throw new Error(
          data.error ||
            "Failed to generate pallet arrangement."
        );
      }

      /*
       * Save the REAL output
       * returned by the C++ algorithm.
       */

      setResult(data);

      /*
       * Reset animation state.
       */

      setAnimationState({
        placedCount: 0,
        playing: false,
      });

      setAnimationCommand({
        type: "reset",
        id: Date.now(),
      });

      setAnimationKey(
        (previous) => previous + 1
      );

      /*
       * Move to visualization page.
       */

      setPage("visualization");
    } catch (error) {
      console.error(error);

      setError(
        error.message ||
          "Something went wrong while running the palletization algorithm."
      );
    } finally {
      setLoading(false);
    }
  };

  const handlePlay = () => {
    setAnimationCommand({
      type: "play",
      id: Date.now(),
    });
  };

  const handlePause = () => {
    setAnimationCommand({
      type: "pause",
      id: Date.now(),
    });
  };

  const handleReset = () => {
    setAnimationCommand({
      type: "reset",
      id: Date.now(),
    });
  };

  const handleAnimationState =
    useCallback((state) => {
      setAnimationState((previous) => {
        if (
          previous.placedCount ===
            state.placedCount &&
          previous.playing ===
            state.playing
        ) {
          return previous;
        }

        return state;
      });
    }, []);

  const totalBoxes =
    result?.placements?.length ||
    form.boxCount;

  const isComplete =
    animationState.placedCount >=
    totalBoxes;

  /*
   * =====================================
   * HOME PAGE
   * =====================================
   */

  if (page === "home") {
    return (
      <div className="app">

        <header className="header">

          <div className="header-content">

            <p className="eyebrow">
              PALLETIZATION SYSTEM
            </p>

            <h1>
              Optimized Pallet Arrangement
            </h1>

            <p className="subtitle">
              Configure your boxes and pallet dimensions,
              then generate an optimized three-dimensional
              pallet arrangement.
            </p>

          </div>

        </header>

        <main className="home-main">

          <section className="input-panel">

            <div className="section-heading">

              <p className="section-label">
                CONFIGURATION
              </p>

              <h2>
                Input Parameters
              </h2>

              <p>
                Enter the box and pallet dimensions to
                generate the optimized arrangement.
              </p>

            </div>

            <form onSubmit={handleSubmit}>

              <div className="input-group full-width">

                <label htmlFor="boxCount">
                  Number of Boxes
                </label>

                <input
                  id="boxCount"
                  name="boxCount"
                  type="number"
                  min="1"
                  value={form.boxCount}
                  onChange={handleChange}
                />

              </div>

              <div className="dimension-section">

                <div className="dimension-heading">

                  <span>
                    Box Dimensions
                  </span>

                  <small>
                    Length × Width × Height
                  </small>

                </div>

                <div className="dimension-grid">

                  <div className="input-group">

                    <label htmlFor="boxLength">
                      Length
                    </label>

                    <input
                      id="boxLength"
                      name="boxLength"
                      type="number"
                      min="1"
                      value={form.boxLength}
                      onChange={handleChange}
                    />

                  </div>

                  <div className="input-group">

                    <label htmlFor="boxWidth">
                      Width
                    </label>

                    <input
                      id="boxWidth"
                      name="boxWidth"
                      type="number"
                      min="1"
                      value={form.boxWidth}
                      onChange={handleChange}
                    />

                  </div>

                  <div className="input-group">

                    <label htmlFor="boxHeight">
                      Height
                    </label>

                    <input
                      id="boxHeight"
                      name="boxHeight"
                      type="number"
                      min="1"
                      value={form.boxHeight}
                      onChange={handleChange}
                    />

                  </div>

                </div>

              </div>

              <div className="dimension-section">

                <div className="dimension-heading">

                  <span>
                    Pallet Dimensions
                  </span>

                  <small>
                    Length × Width × Height
                  </small>

                </div>

                <div className="dimension-grid">

                  <div className="input-group">

                    <label htmlFor="palletLength">
                      Length
                    </label>

                    <input
                      id="palletLength"
                      name="palletLength"
                      type="number"
                      min="1"
                      value={form.palletLength}
                      onChange={handleChange}
                    />

                  </div>

                  <div className="input-group">

                    <label htmlFor="palletWidth">
                      Width
                    </label>

                    <input
                      id="palletWidth"
                      name="palletWidth"
                      type="number"
                      min="1"
                      value={form.palletWidth}
                      onChange={handleChange}
                    />

                  </div>

                  <div className="input-group">

                    <label htmlFor="palletHeight">
                      Height
                    </label>

                    <input
                      id="palletHeight"
                      name="palletHeight"
                      type="number"
                      min="1"
                      value={form.palletHeight}
                      onChange={handleChange}
                    />

                  </div>

                </div>

              </div>

              {error && (
                <div className="error-message">
                  {error}
                </div>
              )}

              <button
                className="optimize-button"
                type="submit"
                disabled={loading}
              >
                {loading
                  ? "Generating Arrangement..."
                  : "Optimize & Visualize"}
              </button>

            </form>

          </section>

        </main>

      </div>
    );
  }

  /*
   * =====================================
   * VISUALIZATION PAGE
   * =====================================
   */

  return (
    <div className="visualization-page">

      <header className="visualization-page-header">

        <div>

          <button
            className="back-button"
            type="button"
            onClick={() => {
              setPage("home");
            }}
          >
            Back to Configuration
          </button>

          <div className="visualization-title">

            <p className="eyebrow">
              3D VISUALIZATION
            </p>

            <h1>
              Optimized Pallet Arrangement
            </h1>

          </div>

        </div>

        <div
          className={`status-badge ${
            animationState.playing
              ? "status-playing"
              : isComplete
              ? "status-complete"
              : ""
          }`}
        >
          {animationState.playing
            ? "Playing"
            : isComplete
            ? "Complete"
            : "Ready"}
        </div>

      </header>

      <main className="visualization-main">

        <section className="scene-section">

          <div className="scene-wrapper">

            <PalletScene
              key={animationKey}
              result={result}
              command={animationCommand}
              onStateChange={
                handleAnimationState
              }
            />

          </div>

        </section>

        <section className="visualization-controls">

          <div className="animation-info">

            <span>
              BOXES PLACED
            </span>

            <strong>
              {Math.min(
                animationState.placedCount,
                totalBoxes
              )}{" "}
              / {totalBoxes}
            </strong>

          </div>

          <div className="control-buttons">

            <button
              type="button"
              className="control-button play-button"
              onClick={handlePlay}
              disabled={
                animationState.playing
              }
            >
              Play
            </button>

            <button
              type="button"
              className="control-button pause-button"
              onClick={handlePause}
              disabled={
                !animationState.playing
              }
            >
              Pause
            </button>

            <button
              type="button"
              className="control-button reset-button"
              onClick={handleReset}
            >
              Reset
            </button>

          </div>

        </section>

        {result?.statistics && (
          <section className="statistics-section">

            <div className="stat-card">

              <span>
                Total Boxes
              </span>

              <strong>
                {result.statistics.totalBoxes}
              </strong>

            </div>

            <div className="stat-card">

              <span>
                Full Pallets
              </span>

              <strong>
                {result.statistics.fullPallets}
              </strong>

            </div>

            <div className="stat-card">

              <span>
                Last Pallet Utilization
              </span>

              <strong>
                {
                  result.statistics
                    .lastPallet
                    ?.utilization
                }%
              </strong>

            </div>

          </section>
        )}

      </main>

    </div>
  );
}

export default App;