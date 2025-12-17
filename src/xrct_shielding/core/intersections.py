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
    ray_origin_x = ray.origin.x
    ray_origin_y = ray.origin.y
    ray_target_x = ray.target.x
    ray_target_y = ray.target.y

    ray_dir_x = ray_target_x - ray_origin_x
    ray_dir_y = ray_target_y - ray_origin_y

    ray_length = math.hypot(ray_dir_x, ray_dir_y)
    if ray_length == 0.0:
        return 0.0
    
    # normalizing ray direction
    ray_dir_x /= ray_length
    ray_dir_y /= ray_length

    wall_start_x = wall.segment.p1.x
    wall_start_y = wall.segment.p1.y
    wall_end_x = wall.segment.p2.x
    wall_end_y = wall.segment.p2.y

    wall_dir_x = wall_end_x - wall_start_x
    wall_dir_y = wall_end_y - wall_start_y

    determinant = ray_dir_x * wall_dir_y - ray_dir_y * wall_dir_x

    # Parallel (or nearly parallel) ray and wall → no intersection
    if abs(determinant) < 1e-12:
        return 0.0

    ray_param = (
        (wall_start_x - ray_origin_x) * wall_dir_y
        - (wall_start_y - ray_origin_y) * wall_dir_x
    ) / determinant

    wall_param = (
        (wall_start_x - ray_origin_x) * ray_dir_y
        - (wall_start_y - ray_origin_y) * ray_dir_x
    ) / determinant

    if ray_param < 0.0 or wall_param < 0.0 or wall_param > 1.0:
        return 0.0

    wall_length = math.hypot(wall_dir_x, wall_dir_y)
    if wall_length == 0.0:
        return 0.0  # Degenerate wall segment
    
    wall_normal_x = -wall_dir_y / wall_length
    wall_normal_y = wall_dir_x / wall_length

    cos_theta = abs(ray_dir_x * wall_normal_x + ray_dir_y * wall_normal_y)
    if cos_theta < 1e-12:
        return 0.0
    
    return wall.thickness / cos_theta