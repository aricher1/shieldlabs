import json
import matplotlib.pyplot as plt


SCENE_PATH = "cpp/build/scene.json"
GRID_SPACING_CM = 50     # adjust for visibility


def main():
    with open(SCENE_PATH, "r") as f:
        scene = json.load(f)

    fig, ax = plt.subplots()

    # Plot walls
    for w in scene["walls"]:
        x = [w["a"][0], w["b"][0]]
        y = [w["a"][1], w["b"][1]]
        ax.plot(x, y, linewidth = 3)

    # Grid
    ax.set_aspect("equal", adjustable = "box")

    xmin, xmax = ax.get_xlim()
    ymin, ymax = ax.get_ylim()

    ax.set_xticks(
        range(int(xmin // GRID_SPACING_CM * GRID_SPACING_CM),
              int(xmax + GRID_SPACING_CM),
              GRID_SPACING_CM)
    )

    ax.set_yticks(
        range(int(ymin // GRID_SPACING_CM * GRID_SPACING_CM),
              int(ymax + GRID_SPACING_CM),
              GRID_SPACING_CM)
    )

    ax.grid(True)
    ax.set_xlabel("cm")
    ax.set_ylabel("cm")
    ax.set_title("XRCT Geometry Debug View")

    plt.show()


if __name__ == "__main__":
    main()