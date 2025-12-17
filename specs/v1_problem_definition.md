# v1 Problem Definition

## Objective
Compute photon dose rate at a point in 2D space from point sources,
accounting for attenuation and buildup through planar shielding walls.

## Physics Included
- Photon radiation (gamma / X-ray)
- Inverse square law
- Exponential attenuation
- Buildup factor
- Linear superposition of photon energies

## Physics Excluded
- Skyshine
- Room scatter
- Energy spectrum hardening
- Secondary radiation
- Neutrons
- Time dependence

## Geometry Assumptions
- 2D planar geometry
- Walls are straight line segments
- Walls have finite thickness
- Walls are infinite in height
- Rays travel in straight lines

## Inputs
- Source position (x, y)
- Source energy spectrum
- Wall geometry and material properties
- Evaluation point (x, y)

## Output
- Dose rate at point (units to be finalized)

## Validation Plan
- Hand calculation: inverse-square only
- Single slab attenuation comparison
- Multi-slab attenuation comparison
- Published shielding examples
