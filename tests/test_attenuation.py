from xrct_shielding.physics.attenuation import attenuate
import math



def test_single_material_attenuation():

    lengths = {"lead": 0.5}
    coeffs = {"lead": 2.0}

    expected = math.exp(-2.0 * 0.5)
    
    assert math.isclose(attenuate(lengths, coeffs), expected)


def test_multiple_attenuation():

    lengths = {
        "lead": 0.5,
        "concrete": 1.0,
    }

    coeffs = {
        "lead": 2.0,
        "concrete": 0.2,
    }

    expected = math.exp(-(2.0 * 0.5 + 0.2 * 1.0))

    assert math.isclose(attenuate(lengths, coeffs), expected)


def test_empty_materials_returns_unity():
    
    assert attenuate({}, {}) == 1.0