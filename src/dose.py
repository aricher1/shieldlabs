from core.geometry import Point, Wall
from core.sources import Source
from core.rays import Ray

def dose_at_point(source, point, walls):
    """
    Compute photon dose rate at a point from a source through walls.

    This is the atomic physics function.
    All higher-level features must call this.
    """
    raise NotImplementedError("Physics kernel not implemented yet")

