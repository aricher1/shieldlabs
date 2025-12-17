from src.core.geometry import Point, Segment, Wall
from src.core.rays import Ray
from src.core.intersections import wall_path_length
import math




def test_perpendicular_hit():
    # Ray hits wall at 90 degrees
    # Path length should equal wall thickness

    wall = Wall(
            segment = Segment(Point(0, 0), Point(0, 10)),
            thickness = 0.2,
            material = "lead"
        )

    ray = Ray(Point(-5, 5), Point(5, 5))

    assert math.isclose(wall_path_length(ray, wall), 0.2)


def test_angled_hit():
    # Ray hits wall at angle
    # Path length should be thickness / cos(theta)
    
    wall = Wall(
        segment = Segment(Point(0, 0), Point(0, 10)),
        thickness = 0.2,
        material = "lead"
    )

    ray = Ray(Point(-5, 0), Point(5, 10))

    excepted = 0.2 / math.cos(math.pi / 4)

    assert math.isclose(wall_path_length(ray, wall), excepted, rel_tol = 1e-6)


def test_ray_misses_wall():
    # No intersection -> pass length = 0
    
    wall = Wall(
        segment = Segment(Point(0, 0), Point(0, 10)),
        thickness = 0.2,
        material = "lead"
    )

    ray = Ray(Point(-5, 5), Point(-5, 5))

    assert wall_path_length(ray, wall) == 0.0
