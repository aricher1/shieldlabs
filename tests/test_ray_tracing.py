from xrct_shielding.core.geometry import Point, Segment, Wall
from xrct_shielding.core.rays import Ray
from xrct_shielding.core.ray_tracing import ray_material_path_lengths




def test_single_wall_single_material():
    
    wall = Wall(
        segment = Segment(Point(0, 0), Point(0, 10)),
        thickness = 0.2,
        material = "lead" 
    )

    ray = Ray(Point(-5, 5), Point(5, 5))

    result = ray_material_path_lengths(ray, [wall])

    assert result == {"lead": 0.2}


def test_two_walls_same_material_accumulate():
    
    walls = [
        Wall(Segment(Point(0, 0), Point(0, 10)), 0.2, "lead"),
        Wall(Segment(Point(2, 0), Point(2, 10)), 0.3, "lead"),
    ]

    ray = Ray(Point(-5, 5), Point(5, 5))

    result = ray_material_path_lengths(ray, walls)

    assert result == {"lead": 0.5}


def test_two_walls_different_materials():
    
    walls = [
        Wall(Segment(Point(0, 0), Point(0, 10)), 0.2, "lead"),
        Wall(Segment(Point(2, 0), Point(2, 10)), 1.0, "concrete"),
    ]

    ray = Ray(Point(-5, 5), Point(5, 5))

    result = ray_material_path_lengths(ray, walls)

    assert result == {"lead": 0.2, "concrete": 1.0}


def test_ray_misses_all_walls():

    wall = Wall(
        segment = Segment(Point(0, 0), Point(0, 10)),
        thickness = 0.2,
        material = "lead"
    )

    ray = Ray(Point(-5, -5), Point(-5, -3))

    result = ray_material_path_lengths(ray, [wall])

    assert result == {}