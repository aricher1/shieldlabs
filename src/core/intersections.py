from .rays import Ray
from .geometry import Wall


def wall_path_length(ray: Ray, wall: Wall) -> float:
    """
    Returns effective path length of the ray through the wall using an analytical slab model.

    Returns 0.0 if the ray does not intersect the wall.
    """
    # 1. Check if ray intersects wall centerline segment
    # 2. Compute angle between ray and wall normal
    # 3. Project thickness along ray
    raise NotImplementedError

