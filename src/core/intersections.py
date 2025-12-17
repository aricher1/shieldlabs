from .rays import Ray
from .geometry import Wall


def wall_path_length(ray: Ray, wall: Wall) -> float:
    """
    Returns the path of the length of the ray through the wall.
    Returns 0.0 if there is no intersection.
    """
    raise NotImplementedError

