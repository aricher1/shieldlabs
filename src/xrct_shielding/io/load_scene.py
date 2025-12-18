import json
from xrct_shielding.core.geometry import Point, Segment, Wall



def load_scene(path : str):
    with open(path, "r") as f:
        data = json.load(f)


    walls = []
    for w in data["walls"]:
        a = Point(*w["a"])
        b = Point(*w["b"])
        walls.append(
            Wall(
                segment = Segment(a, b),
                thickness = w["thickness"],
                material = "lead"      # temporary mapping
            )
        )
    
    sources = [Point(*p) for p in data["sources"]]
    
    evaluation_points = [Point(*p) for p in data["evaluation_points"]]

    return walls, sources, evaluation_points