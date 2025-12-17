from dataclasses import dataclass


@dataclass(frozen = True)
class Material:
    name: str
    density: float      # kg/m^3

