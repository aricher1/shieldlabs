from typing import Dict
import math



def attenuate(
        material_lengths: Dict[str, float],
        attenuation_coeffs: Dict[str, float],
) -> float:
    """
    Compute transmission through multiple materials using exponential attenuation.

    Returns:
        Transmission factor in (0, 1], unitless.
    """

    exponent = 0.0

    for material, length in material_lengths.items():
        mu = attenuation_coeffs[material]
        exponent += mu * length


    return math.exp(-exponent)