from xrct_shielding.io.load_scene import load_scene
from xrct_shielding.solver.dose_solver import dose_at_point
from xrct_shielding.core.geometry import Point
import math



def test_cpp_geometry_pipeline():

    walls, sources, _ = load_scene("cpp/build/scene.json")

    source = Point(-500, 150)
    point = Point(100, 150)

    attenuation = {"lead": 1.0}

    dose = dose_at_point(
        source_position = source,
        evaluation_point = point,
        walls = walls,
        attenuation_coeffs = attenuation,  
    )

    assert dose > 0.0