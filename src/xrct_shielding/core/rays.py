from dataclasses import dataclass
from .geometry import Point


@dataclass(frozen = True)
class Ray:
    origin: Point
    target: Point
    