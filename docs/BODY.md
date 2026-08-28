# The body

One rigid segment: a regular icosahedron, near-black mirror faces, a green neon tube along every
edge. This is what the worm brings to the Grid in `program_rez` - the Grid decides the senses,
the Program brings the body - and this page carries the numbers and the why.

The body is vanilla C++20 in `src/worm/body.{hpp,cpp}`: arithmetic from the golden ratio and a
fixed orientation, built once at first use, lent to the Grid through a `TglRenderModel` whose
arrays point at the body's own storage. The Grid copies before `program_rez` returns, exactly as
the ABI states, so nothing is handed over and nothing is freed.

## Why an icosahedron

The worm's segments could have been dodecahedra - both are Platonic, both read as a "segment"
when strung in a line. The icosahedron won on three counts:

- **Every face is a triangle.** The wire carries triangles; a dodecahedron's twelve pentagons
  would be fanned into thirty-six triangles with interior edges that are not edges of the solid,
  and the neon would need to know which edges are real. The icosahedron's twenty faces go on the
  wire as twenty triangles and its thirty edges are exactly the thirty tubes.
- **It rests on a face.** Face 0 is turned to face -Y, so the body sits on a triangle, not on a
  point - Master Control stands a body on its lowest vertex, and the tubes along that face's
  edges are what touch the floor.
- **It is nearly a sphere.** Twelve vertices on one circumsphere, the roundest of the five: a
  segment that will later roll and bend without any one face dominating the look.

The dodecahedron stays the recorded alternative in `TODO.md`; swapping is a matter of the vertex
and face tables.

## The numbers

| Quantity | Value | Why |
|---|---|---|
| `BODY_CIRCUMRADIUS` | 0.25 m | origin to any shell vertex; edge = 0.263 m, body ≈ 0.5 m across |
| `NEON_RADIUS` | 0.008 m | the tube's radius, a visible line rather than a hair |
| `NEON_PROUD` | 0.004 m | the tube's centre line stands this far off the edge, outward |
| Shell | 12 vertices, 20 triangles | material 0 |
| Neon | 30 edges × (6 vertices, 6 triangles) | material 1, a triangular prism per edge |
| Joint stubs | 2 × (6 vertices, 6 triangles) | material 1, the same prism from the nose spike, and from its antipode, to the joint tips on the axis |
| Total | 204 vertices, 212 triangles, 2 materials | under the wire's caps of 1024 / 2048 / 16 |
| `BODY_SEGMENTS` | 8 | the chain, the head counted: the wire's cap and a worm's worth |
| `JOINT_STUB_LENGTH` | 0.03 m | how far past the circumradius a joint tip lies |
| `JOINT_TIP_REACH` | 0.28 m | the joint tips, on the axis: `(0, 0, ±0.28)`; two segments meet at one, a pivot |
| `SEGMENT_SPACING` | 0.56 m | twice the reach - tip to tip through one segment |
| Extent | 0.25 m | under the world's `BODY_MAX_EXTENT` of 4 m |

The first body's eyes sit at body-frame `(0, 0, -0.2)` and its ears at `(0, 0, 0.2)`; a
quarter-metre circumradius puts both just inside the shell, the eyes behind the nose.

## Orientation

The twelve raw vertices are the corners of three mutually perpendicular golden rectangles,
`(0, ±1, ±φ)` and its two cyclic permutations, scaled by `0.25 / √(1 + φ²)`. Then two turns:

1. **Face down.** The outward normal of face 0 is rotated onto `-Y` (Rodrigues about the axis
   perpendicular to both), so the body rests on that face.
2. **Nose forward.** Of the vertices around the waist (`|y| < R/2`), the one furthest from the
   vertical axis is turned about `Y` exactly onto `-Z` - the direction the eyes look. A turn about
   the vertical keeps the resting face flat.

Both turns are deterministic arithmetic on the same constants; `Body::theWorm()` builds once and
the tests check the same bytes come back every time.

## Winding and materials

Every face is wound counter-clockwise seen from outside, so `cross(edge1, edge2)` points away
from the origin and the tracer's two-sided test reads the shell as a surface seen from outside.
The tubes' side quads are wound away from their own centre line for the same reason. The tests
check every triangle has area (the Grid refuses a degenerate one) and every shell normal points
outward.

| Material | Colour | IOR | Emission | Transmission | Reads as |
|---|---|---|---|---|---|
| 0 shell | (0.06, 0.08, 0.07) | 2.4 | 0 | 0 | a near-black mirror with a green cast, the floor's reflectivity |
| 1 neon | (0.05, 0.55, 0.20) | 1.5 | (0.30, 4.20, 1.20) | 0 | green neon at the Grid's own neon intensities (its primary is (0.10, 2.60, 4.20)) |

The emission intensities are above 1 on purpose: the Grid's tone mapping expects neon to bloom,
and a tube that emits at 1.0 reads as a painted stripe.

## The chain

The owner's ruling (2026-08-26): a worm is a chain of icosahedra joined spike to spike, and it
undulates. This body is one segment of eight. The joint is authored here rather than carried on
the wire: a neon stub - the same triangular prism the edges wear - runs out of the **nose
spike** (the waist vertex on -Z as seen from above, where the eyes look) and out of its
**antipode** (the tail spike) to a **joint tip that lies exactly on the body's axis**,
`JOINT_TIP_REACH` = 0.25 + 0.03 = 0.28 m from the origin: `(0, 0, -0.28)` for the nose,
`(0, 0, +0.28)` for the tail. Two consecutive segments meet at one such tip - a pivot the chain
bends around - so the spacing the model declares is twice the reach: `SEGMENT_SPACING` = 0.56 m.

On the axis, deliberately. An icosahedron resting on a face has no vertex on its own horizontal
axis: the waist vertices sit 0.1876 circumradii (4.7 cm) above or below it, 10.8° off. The
first authoring ran each stub along its spike's own direction, so the nose tip ended 4.7 cm
above the axis and the tail tip 4.7 cm below, and two neighbours' tips stood
0.56 × |n + Z| = 10.5 cm apart even on a straight chain - the gap the owner saw between every
pair (2026-08-28). The stub now kinks by those 10.8° to reach the axis; the resting posture,
the eyes and the ears are untouched.

Where the trailing segments stand is the world's business (Master Control places them along the
path the head walked - kinematic trail, not articulation) and the Grid's to draw (the mesh once
per segment). The Program is told nothing of where its tail is: the senses are the head's.

## What the world sees

Master Control builds the hull from every vertex, so the tubes' rails - proud of the shell by
`NEON_PROUD + NEON_RADIUS` - are the hull's extremes, and the body stands on a tube's outermost
rail, `lowest()` just below the resting face. That is by design: a worm lies on its tubes.

## Not yet

- The undulation is whatever the User weaves; a lateral wave as a function of speed is owed in
  Master Control, authored and said so, once the following looks right in the window.
- No animation of the body: `program_tick` answers zeroes until the panel gives the worm a will.
