from dataclasses import dataclass
import math


@dataclass(frozen = True)
class Point:
    x: float
    y: float


@dataclass(frozen = True)
class Segment:
    p1: Point
    p2: Point


@dataclass(frozen = True)
class Wall:
    segment: Segment
    thickness: float
    material: str

