from PIL import Image, ImageDraw
import os

def create_block_texture(name, size, draw_func):
    """Create a block texture with the given name and drawing function"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw_func(draw, size)
    img.save(f'engine/assets/textures/voxels/{name}.png')

def stone_texture(draw, size):
    """Gray with slight noise"""
    for y in range(size):
        for x in range(size):
            # Base gray with slight variation
            gray = 140 + ((x * y) % 20)
            draw.point((x, y), (gray, gray, gray + 5, 255))

def dirt_texture(draw, size):
    """Brown with dark spots"""
    for y in range(size):
        for x in range(size):
            # Brown base with darker spots
            is_spot = ((x + y) % 4) == 0
            r = 115 - (20 if is_spot else 0)
            g = 77 - (15 if is_spot else 0)
            b = 46 - (10 if is_spot else 0)
            draw.point((x, y), (r, g, b, 255))

def grass_texture(draw, size):
    """Green with brighter blades"""
    for y in range(size):
        for x in range(size):
            # Green with some bright blades
            is_blade = ((x * 7 + y * 3) % 5) == 0
            r = 60
            g = 150 + (40 if is_blade else 0)
            b = 70
            draw.point((x, y), (r, g, b, 255))

def wood_texture(draw, size):
    """Brown with vertical stripes"""
    for y in range(size):
        for x in range(size):
            # Vertical stripes
            is_stripe = (x // 2) % 2 == 0
            r = 120 if is_stripe else 100
            g = 85 if is_stripe else 70
            b = 50 if is_stripe else 40
            draw.point((x, y), (r, g, b, 255))

def leaves_texture(draw, size):
    """Green with alpha cutouts"""
    for y in range(size):
        for x in range(size):
            # Green with holes
            is_hole = ((x + y) % 4) == 0
            r = 50
            g = 140
            b = 60
            a = 0 if is_hole else 255
            draw.point((x, y), (r, g, b, a))

# Create output directory if it doesn't exist
os.makedirs('engine/assets/textures/voxels', exist_ok=True)

# Generate all block textures
size = 16  # 16x16 textures
create_block_texture('stone', size, stone_texture)
create_block_texture('dirt', size, dirt_texture)
create_block_texture('grass', size, grass_texture)
create_block_texture('wood', size, wood_texture)
create_block_texture('leaves', size, leaves_texture)

print("Block textures generated in engine/assets/textures/voxels/")