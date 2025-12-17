from dataclasses import dataclass
from .geometry import Point


@dataclass(frozen = True)
class PhotonEnergy:
    energy_keV: float
    intensity: float        # relative or absolute


@dataclass(frozen = True)
class Source:
    position: Point
    spectrum: list[PhotonEnergy]