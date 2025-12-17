from .rays import Ray
from .geometry import Wall
import math




def wall_path_length(ray: Ray, wall: Wall) -> float:
    """
    Returns effective path length of the ray through the wall using an analytical slab model.

    Returns 0.0 if the ray does not intersect the wall.
    """
    # 1. Check if ray intersects wall centerline segment
    # 2. Compute angle between ray and wall normal
    # 3. Project thickness along ray

    # guard against degenerate ray
    ox, oy = ray.origin.x, ray.origin.y
    tx, ty = ray.target.x, ray.target.y

    dx = tx - ox
    dy = ty - oy

    ray_len = math.hypot(dx, dy)
    if ray_len == 0.0:
        return 0.0
    
    # normalizing ray direction
    dx /= ray_len
    dy /= ray_len

    x1, y1 = wall.segment.p1.x, wall.segment.p1.y
    x2, y2 = wall.segment.p2.x, wall.segment.p2.y
    
    sx = x2 - x1
    sy = y2 - y1

    denom = dx * sy - dy * sx
    if abs(denom) < 1e-12:  # ray is parallel to wall
        return 0.0

    t = ((x1 - ox) * sy - (y1 - oy) * sx) / denom
    u = ((x1 - ox) * dy - (y1 - oy) * dx) / denom

    if t < 0.0 or u < 0.0 or u > 1.0:
        return 0.0 

    wall_len = math.hypot(sx, sy)
    if wall_len == 0.0:
        return 0.0
    
    nx = -sy / wall_len
    ny = sx / wall_len

    cos_theta = abs(dx * nx + dy * ny)
    if cos_theta < 1e-12:
        return 0.0
    
    return wall.thickness / cos_theta