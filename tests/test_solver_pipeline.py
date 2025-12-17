from xrct_shielding.core.geometry import Point, Segment, Wall
from xrct_shielding.solver.dose_solver import dose_at_point
import math




def test_solver_pipeline_single_wall():

    walls = [
        Wall(
            segment = Segment(Point(0, 0), Point(0, 10)),
            thickness = 0.5,
            material = "lead"
        )
    ]

    attenuation_coeffs = {
        "lead": 2.0
    }

    source = Point(-5, 5)
    point = Point(5, 5)

    result = dose_at_point(
        source_position = source,
        evaluation_point = point,
        walls = walls,
        attenuation_coeffs = attenuation_coeffs,
    )

    expeected = math.exp(-2.0 * 0.5)

    assert math.isclose(result, expeected)