import argparse
from PIL import Image

def generate_fuzzball(diameter: int, base_color: int, alpha_start: int, alpha_end: int) -> list[int]:
    """
    Generate a fuzzball based on parameters.
    Returns a one dimensional array representing the fuzzball
    with hexadecimal color values.
    """
    assert diameter > 0, "Diameter must be greater than 0"
    assert 0 <= alpha_start <= 255, "Alpha start must be between 0 and 255"
    assert 0 <= alpha_end <= 255, "Alpha end must be between 0 and 255"
    assert 0x000000 <= base_color <= 0xFFFFFF, "Base color must be a valid hex color (0x000000 to 0xFFFFFF)"

    print("Generating fuzzball...")

    fuzzball = list()
    center = (diameter - 1) / 2.0
    radius = diameter / 2.0
  
    for y in range(diameter):
        for x in range(diameter):
            # calc distance from center
            dist_x = x - center
            dist_y = y - center
            distance = (dist_x**2 + dist_y**2)**0.5

            if distance <= radius:
                # exponential alpha falloff
                t = (distance / radius)**2
                alpha = int(alpha_start - t * (alpha_start - alpha_end))
                color = (alpha << 24) | (base_color & 0xFFFFFF)  # alpha + rgb
                fuzzball.append(color)
            else:
                # outside the circle, fully transparent
                fuzzball.append(0x00000000)

    print("Fuzzball generation complete.")
    return fuzzball

def preview(fuzzball: list[int], diameter: int):
    """
    Preview the generated fuzzball using PIL.
    """
    assert len(fuzzball) == diameter * diameter, "Fuzzball size does not match diameter"

    image = Image.new("RGBA", (diameter, diameter))
    pixels = image.load()

    for y in range(diameter):
        for x in range(diameter):
            color = fuzzball[y * diameter + x]

            # decompose color
            a = (color >> 24) & 0xFF
            r = (color >> 16) & 0xFF
            g = (color >> 8) & 0xFF
            b = color & 0xFF        

            # set pixel color
            pixels[x, y] = (r, g, b, a)

    image.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate a fuzzball based on parameters as a hex array.")
    parser.add_argument("-d", "--diameter", type=int, help="Diameter of the fuzzball", required=True)
    parser.add_argument("-c" ,"--base_color", type=lambda x: int(x, 16), default=0xFFFFFF, help="Base color in hex (e.g., 0xFF0000 for red)")
    parser.add_argument("-s" ,"--alpha_start", type=int, default=255, help="Starting alpha value (0-255)")
    parser.add_argument("-e" ,"--alpha_end", type=int, default=0, help="Ending alpha value (0-255)")
    parser.add_argument("-p", "--preview", action="store_true", help="Preview the generated fuzzball")
    
    args = parser.parse_args()

    fuzzball = generate_fuzzball(args.diameter, args.base_color, args.alpha_start, args.alpha_end)
    print(f"Fuzzball ({args.diameter} x {args.diameter}):")
    print(", ".join(f"0x{c:08X}" for c in fuzzball))

    if args.preview:
        preview(fuzzball, args.diameter)
