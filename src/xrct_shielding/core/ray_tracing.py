from typing import Dict, List
from .rays import Ray
from .geometry import Wall
from .intersections import wall_path_length




def ray_material_path_lengths(ray: Ray, walls: list[Wall]) -> dict[str, float]:
    """
    Returns total path length per material along the ray.
    Example:
        {"lead": 0.42, "concrete": 1.3}
    """

    material_lengths: Dict[str, float] = {}

    for wall in walls:
        length = wall_path_length(ray, wall)
        if length > 0.0:
            material_lengths[wall.material] = (
                material_lengths.get(wall.material, 0.0) + length
            )
    
    
    return material_lengths