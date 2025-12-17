from xrct_shielding.core.geometry import Point, Wall
from xrct_shielding.core.rays import Ray
from xrct_shielding.core.ray_tracing import ray_material_path_lengths
from xrct_shielding.physics.attenuation import attenuate




def dose_at_point(
        source_position: Point,
        evaluation_point: Point,
        walls: list[Wall],
        attenuation_coeffs: dict[str, float],
) -> float:
    """
    High-level solver pipeline.

    This function orchestrates geometry and physics to produce a scalar dose-like quantity at a point.

    Notes:
    - Inverse-square falloff is not applied yet.
    - Energy spectra are not applied yet.
    - Returned value is a relative dose proxy.
    """

    ray = Ray(origin = source_position, target = evaluation_point)

    material_lengths = ray_material_path_lengths(ray, walls)

    transmission = attenuate(material_lengths, attenuation_coeffs)

    return transmission