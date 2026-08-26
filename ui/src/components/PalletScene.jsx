import {
  useEffect,
  useMemo,
  useRef,
  useState,
} from "react";

import {
  Canvas,
  useFrame,
} from "@react-three/fiber";

import {
  OrbitControls,
  PerspectiveCamera,
  Html,
  Text,
} from "@react-three/drei";

import * as THREE from "three";

const SCALE = 0.01;

/*
 * Animation speed.
 *
 * Increase this number if you want
 * boxes to move even faster.
 */
const ANIMATION_SPEED = 7;

/*
 * ---------------------------------
 * AXES
 * X = GREEN
 * Y = BLUE
 * Z = RED
 * ---------------------------------
 */

function AxisGuides({
  size = 8,
}) {
  return (
    <group>

      {/* X AXIS - GREEN */}

      <arrowHelper
        args={[
          new THREE.Vector3(1, 0, 0),
          new THREE.Vector3(0, 0, 0),
          size,
          "#22c55e",
          0.35,
          0.2,
        ]}
      />

      <Text
        position={[
          size + 0.5,
          0,
          0,
        ]}
        fontSize={0.35}
        color="#22c55e"
      >
        X
      </Text>

      {/* Y AXIS - BLUE */}

      <arrowHelper
        args={[
          new THREE.Vector3(0, 1, 0),
          new THREE.Vector3(0, 0, 0),
          size,
          "#3b82f6",
          0.35,
          0.2,
        ]}
      />

      <Text
        position={[
          0,
          size + 0.5,
          0,
        ]}
        fontSize={0.35}
        color="#3b82f6"
      >
        Y
      </Text>

      {/* Z AXIS - RED */}

      <arrowHelper
        args={[
          new THREE.Vector3(0, 0, 1),
          new THREE.Vector3(0, 0, 0),
          size,
          "#ef4444",
          0.35,
          0.2,
        ]}
      />

      <Text
        position={[
          0,
          0,
          size + 0.5,
        ]}
        fontSize={0.35}
        color="#ef4444"
      >
        Z
      </Text>

    </group>
  );
}

/*
 * ---------------------------------
 * PALLET
 * ---------------------------------
 */

function Pallet({
  length,
  width,
  height,
  offset,
}) {
  const palletLength =
    length * SCALE;

  const palletWidth =
    width * SCALE;

  const palletHeight =
    height * SCALE;

  return (
    <group
      position={offset}
    >

      <mesh
        position={[
          palletLength / 2,
          -palletHeight / 2,
          palletWidth / 2,
        ]}
        receiveShadow
      >

        <boxGeometry
          args={[
            palletLength,
            palletHeight,
            palletWidth,
          ]}
        />

        <meshStandardMaterial
          color="#8a623b"
          roughness={0.8}
          metalness={0.05}
        />

      </mesh>

    </group>
  );
}

/*
 * ---------------------------------
 * INDIVIDUAL BOX
 * ---------------------------------
 */

function AnimatedBox({
  box,
  visible,
  isMoving,
  animationFinished,
  onComplete,
}) {
  const meshRef =
    useRef();

  const completedRef =
    useRef(false);

  const [
    selected,
    setSelected,
  ] = useState(false);

  const dimensions =
    box.dimensions;

  /*
   * Algorithm coordinates:
   *
   * C++:
   * position = [x, y, z]
   *
   * Three.js:
   * X = x
   * Y = vertical = z
   * Z = y
   */

  const finalPosition =
    useMemo(() => {
      return [
        box.offset[0] +
          box.position[0] * SCALE +
          (
            dimensions.length *
            SCALE
          ) / 2,

        box.offset[1] +
          box.position[2] * SCALE +
          (
            dimensions.height *
            SCALE
          ) / 2,

        box.offset[2] +
          box.position[1] * SCALE +
          (
            dimensions.width *
            SCALE
          ) / 2,
      ];
    }, [
      box,
      dimensions,
    ]);

  const startPosition =
    useMemo(() => {
      return [
        finalPosition[0],
        finalPosition[1] + 8,
        finalPosition[2],
      ];
    }, [
      finalPosition,
    ]);

  /*
   * Reset position whenever
   * the animation resets.
   */

  useEffect(() => {
    completedRef.current = false;

    if (
      meshRef.current
    ) {
      meshRef.current.position.set(
        startPosition[0],
        startPosition[1],
        startPosition[2]
      );
    }
  }, [
    startPosition,
    box.resetKey,
  ]);

  useFrame(
    (_, delta) => {
      if (
        !meshRef.current ||
        !visible
      ) {
        return;
      }

      const target =
        new THREE.Vector3(
          finalPosition[0],
          finalPosition[1],
          finalPosition[2]
        );

      /*
       * Already placed.
       */

      if (
        animationFinished
      ) {
        meshRef.current.position.copy(
          target
        );

        return;
      }

      /*
       * Current animated box.
       */

      if (
        isMoving
      ) {
        meshRef.current.position.lerp(
          target,
          Math.min(
            delta *
              ANIMATION_SPEED,
            1
          )
        );

        const distance =
          meshRef.current.position
            .distanceTo(target);

        if (
          distance < 0.025 &&
          !completedRef.current
        ) {
          completedRef.current =
            true;

          meshRef.current.position.copy(
            target
          );

          onComplete?.();
        }
      }
    }
  );

  if (
    !visible
  ) {
    return null;
  }

  return (
    <group>

      <mesh
        ref={meshRef}
        castShadow
        receiveShadow
        onClick={(event) => {
          event.stopPropagation();

          setSelected(
            (previous) =>
              !previous
          );
        }}
      >

        <boxGeometry
          args={[
            dimensions.length * SCALE,
            dimensions.height * SCALE,
            dimensions.width * SCALE,
          ]}
        />

        <meshStandardMaterial
          color={
            selected
              ? "#f1d67b"
              : "#d4a017"
          }
          roughness={0.6}
          metalness={0.05}
        />

      </mesh>

      {selected && (
        <Html
          position={[
            finalPosition[0],
            finalPosition[1] +
              (
                dimensions.height *
                SCALE
              ) / 2 +
              0.25,
            finalPosition[2],
          ]}
          center
          distanceFactor={10}
        >

          <div
            style={{
              pointerEvents: "none",
              padding:
                "7px 10px",
              borderRadius: "7px",
              background:
                "rgba(10,11,15,0.92)",
              border:
                "1px solid rgba(255,255,255,0.15)",
              color: "#f4f4f5",
              fontFamily:
                "Inter, system-ui, sans-serif",
              fontSize: "11px",
              fontWeight: "700",
              whiteSpace:
                "nowrap",
              boxShadow:
                "0 8px 25px rgba(0,0,0,0.35)",
            }}
          >
            Box {box.id}

            <div
              style={{
                marginTop:
                  "3px",
                color:
                  "#9ca3af",
                fontSize:
                  "9px",
                fontWeight:
                  "500",
              }}
            >
              Pallet {box.palletId}
            </div>

          </div>

        </Html>
      )}

    </group>
  );
}

/*
 * ---------------------------------
 * SCENE CONTENT
 * ---------------------------------
 */

function SceneContent({
  result,
  command,
  onStateChange,
}) {
  const [
    placedCount,
    setPlacedCount,
  ] = useState(0);

  const [
    playing,
    setPlaying,
  ] = useState(false);

  const [
    movingIndex,
    setMovingIndex,
  ] = useState(null);

  const resetKeyRef =
    useRef(0);

  const placements =
    result?.placements || [];

  const boxDimensions =
    result?.box
      ? {
          length:
            result.box.length,
          width:
            result.box.width,
          height:
            result.box.height,
        }
      : null;

  const palletDimensions =
    result?.pallet
      ? {
          length:
            result.pallet.length,
          width:
            result.pallet.width,
          height:
            result.pallet.height,
        }
      : null;

  /*
   * Get all pallet IDs.
   */

  const palletIds =
    useMemo(() => {
      return [
        ...new Set(
          placements.map(
            (placement) =>
              placement.palletId
          )
        ),
      ];
    }, [
      placements,
    ]);

  /*
   * Give every pallet a separate
   * location in the 3D scene.
   */

  const palletOffsets =
    useMemo(() => {
      const offsets =
        {};

      if (
        !palletDimensions
      ) {
        return offsets;
      }

      const spacing =
        palletDimensions.length *
          SCALE +
        3;

      palletIds.forEach(
        (palletId, index) => {
          offsets[palletId] = [
            index * spacing,
            0,
            0,
          ];
        }
      );

      return offsets;
    }, [
      palletIds,
      palletDimensions,
    ]);

  const boxes =
    useMemo(() => {
      if (
        !boxDimensions
      ) {
        return [];
      }

      return placements.map(
        (placement) => ({
          id:
            placement.boxId,

          palletId:
            placement.palletId,

          position:
            placement.position,

          dimensions:
            boxDimensions,

          offset:
            palletOffsets[
              placement.palletId
            ] || [0, 0, 0],

          resetKey:
            resetKeyRef.current,
        })
      );
    }, [
      placements,
      boxDimensions,
      palletOffsets,
    ]);

  /*
   * Send state to App.jsx.
   */

  useEffect(() => {
    onStateChange?.({
      placedCount,
      playing,
    });
  }, [
    placedCount,
    playing,
    onStateChange,
  ]);

  /*
   * Reset when a new result arrives.
   */

  useEffect(() => {
    resetKeyRef.current += 1;

    setPlacedCount(0);
    setPlaying(false);
    setMovingIndex(null);
  }, [
    result,
  ]);

  /*
   * Handle Play / Pause / Reset.
   */

  useEffect(() => {
    if (
      !command ||
      command.type === "none"
    ) {
      return;
    }

    if (
      command.type === "play"
    ) {
      if (
        placedCount >= boxes.length
      ) {
        resetKeyRef.current += 1;

        setPlacedCount(0);
        setMovingIndex(null);
      }

      setPlaying(true);
    }

    if (
      command.type === "pause"
    ) {
      setPlaying(false);
    }

    if (
      command.type === "reset"
    ) {
      resetKeyRef.current += 1;

      setPlacedCount(0);
      setMovingIndex(null);
      setPlaying(false);
    }
  }, [
    command,
    boxes.length,
  ]);

  /*
   * Start next box.
   */

  useEffect(() => {
    if (
      !playing
    ) {
      return;
    }

    if (
      movingIndex !== null
    ) {
      return;
    }

    if (
      placedCount >= boxes.length
    ) {
      setPlaying(false);
      return;
    }

    setMovingIndex(
      placedCount
    );
  }, [
    playing,
    movingIndex,
    placedCount,
    boxes.length,
  ]);

  const handleBoxComplete =
    () => {
      setPlacedCount(
        (previous) =>
          previous + 1
      );

      setMovingIndex(null);
    };

  /*
   * Calculate scene center so
   * camera controls work properly
   * with multiple pallets.
   */

  const totalSceneLength =
    palletDimensions
      ? (
          palletIds.length *
          (
            palletDimensions.length *
              SCALE +
            3
          )
        )
      : 10;

  const sceneCenter =
    totalSceneLength / 2;

  return (
    <>

<PerspectiveCamera
  makeDefault
  position={[
    sceneCenter + 7,
    8,
    11,
  ]}
  fov={40}
/>

      <ambientLight
        intensity={1.4}
      />

      <directionalLight
        position={[
          12,
          18,
          10,
        ]}
        intensity={2.3}
        castShadow
      />

      <directionalLight
        position={[
          -10,
          8,
          -8,
        ]}
        intensity={0.8}
      />

      <AxisGuides
        size={7}
      />

      {palletDimensions &&
        palletIds.map(
          (palletId) => (
            <Pallet
              key={palletId}
              length={
                palletDimensions.length
              }
              width={
                palletDimensions.width
              }
              height={
                palletDimensions.height
              }
              offset={
                palletOffsets[
                  palletId
                ]
              }
            />
          )
        )}

      {boxes.map(
        (box, index) => {

          const visible =
            index <= placedCount;

          const isMoving =
            index === movingIndex &&
            playing;

          const animationFinished =
            index < placedCount;

          return (
            <AnimatedBox
              key={`${box.id}-${box.palletId}-${resetKeyRef.current}`}
              box={box}
              visible={visible}
              isMoving={isMoving}
              animationFinished={
                animationFinished
              }
              onComplete={
                index === movingIndex
                  ? handleBoxComplete
                  : undefined
              }
            />
          );
        }
      )}

      <gridHelper
        args={[
          Math.max(
            40,
            totalSceneLength + 15
          ),
          40,
          "#343943",
          "#1b1e26",
        ]}
      />

 <OrbitControls
  enableDamping
  dampingFactor={0.08}
  minDistance={5}
  maxDistance={45}
  target={[
    sceneCenter,
    1.5,
    0,
  ]}
/>
    </>
  );
}

/*
 * ---------------------------------
 * MAIN COMPONENT
 * ---------------------------------
 */

function PalletScene({
  result,
  command,
  onStateChange,
}) {
  return (
    <Canvas
      shadows
      gl={{
        antialias: true,
        toneMapping:
          THREE.ACESFilmicToneMapping,
      }}
    >

      <color
        attach="background"
        args={[
          "#090a0e",
        ]}
      />

      <SceneContent
        result={result}
        command={command}
        onStateChange={
          onStateChange
        }
      />

    </Canvas>
  );
}

export default PalletScene;