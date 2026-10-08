# Bricks

The Bricks algorithm builds each layer from complete box footprints in
alternating courses. A course runs along the pallet length and uses either
the box's normal floor orientation or its 90-degree rotation. When course
widths exactly tile the pallet, the algorithm interleaves the two orientations
to create a brick-style pattern and selects a complementary course order for
the next layer.

Before a layout can be used above another layer, every upper box's complete
2D footprint is intersected with the actual footprints in the layer directly
below. The layout is accepted only when the sum of those intersections covers
the upper footprint (within a small floating-point tolerance). Each candidate
layer is also checked for pallet containment and same-layer overlap.

If no complete course pattern is possible, the algorithm falls back to
centered, axis-aligned grids. It can use an upper grid only when the support
check passes; otherwise it repeats a supported layout. This favors physical
support over maximum capacity and may leave pallet area unused for dimensions
that cannot be tiled by complete boxes.