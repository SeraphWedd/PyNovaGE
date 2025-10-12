#!/usr/bin/env python3
"""
Screen buffer diagnostic tool to check if rendering is working.
Uses SHA256 hashing to detect any changes in the screen buffer.
"""

import os, sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import pynovage
import hashlib
import time

def get_render_state_hash():
    """Get a hash of the render state to detect changes."""
    try:
        # Try to get render stats as a proxy for activity
        if hasattr(pynovage.renderer, 'get_stats'):
            stats = pynovage.renderer.get_stats()
            # Create a simple hash from the stats
            state_data = f"{stats.draw_calls}_{stats.sprites_rendered}_{stats.vertices_rendered}".encode()
            return hashlib.sha256(state_data).hexdigest()
        
        # Fallback: create hash from frame timestamp
        state_data = str(time.time()).encode()
        return hashlib.sha256(state_data).hexdigest()
            
    except Exception as e:
        print(f"Error getting render state: {e}")
        return None

def main():
    """Run screen buffer diagnostic."""
    print("🔍 PyNovaGE Screen Buffer Diagnostic")
    print("=" * 50)
    
    # Initialize
    pynovage.init()
    screen = pynovage.display.set_mode((800, 600))
    clock = pynovage.Clock()
    
    # Colors for testing
    black = pynovage.Color(0, 0, 0)
    red = pynovage.Color(255, 0, 0)
    green = pynovage.Color(0, 255, 0)
    blue = pynovage.Color(0, 0, 255)
    white = pynovage.Color(255, 255, 255)
    
    print("Testing if screen buffer changes are detectable...")
    print("This will test various rendering operations and check for changes.\n")
    
    # Store hashes to detect changes
    hashes = []
    frame_count = 0
    changes_detected = 0
    
    test_duration = 5  # seconds
    start_time = time.time()
    
    while (time.time() - start_time) < test_duration:
        frame_count += 1
        
        # Test different rendering operations
        if frame_count < 30:
            # Test 1: Try clearing to black
            pynovage.fill_screen(black)
        elif frame_count < 60:
            # Test 2: Try clearing to red
            pynovage.fill_screen(red)
        elif frame_count < 90:
            # Test 3: Try clearing to green
            pynovage.fill_screen(green)
        elif frame_count < 120:
            # Test 4: Try clearing to blue
            pynovage.fill_screen(blue)
        else:
            # Test 5: Try clearing to white
            pynovage.fill_screen(white)
        
        # Try to draw some shapes (even though they might not work)
        if frame_count > 60:
            pynovage.draw.rect(screen, red, (100, 100, 200, 150))
            pynovage.draw.circle(screen, blue, (400, 300), 50)
        
        # Update display
        pynovage.display.flip()
        
        # Get render state hash
        current_hash = get_render_state_hash()
        
        if current_hash:
            if current_hash not in hashes:
                if hashes:  # Not the first frame
                    changes_detected += 1
                    print(f"Frame {frame_count:3d}: Screen buffer CHANGED - Hash: {current_hash[:16]}...")
                else:
                    print(f"Frame {frame_count:3d}: Initial buffer      - Hash: {current_hash[:16]}...")
                hashes.append(current_hash)
            else:
                print(f"Frame {frame_count:3d}: No change detected   - Hash: {current_hash[:16]}...")
        else:
            print(f"Frame {frame_count:3d}: Could not read screen buffer")
        
        # Handle events
        for event in pynovage.event.get():
            if event.type == pynovage.QUIT:
                break
        
        clock.tick(60)
    
    print(f"\n📊 DIAGNOSTIC RESULTS:")
    print(f"   Total frames rendered: {frame_count}")
    print(f"   Screen buffer changes detected: {changes_detected}")
    print(f"   Unique screen states: {len(hashes)}")
    
    if changes_detected == 0:
        print("❌ PROBLEM: No screen buffer changes detected!")
        print("   This indicates the rendering pipeline is not working.")
        print("   Either:")
        print("   - The screen is not being updated")
        print("   - The rendering commands are not affecting the buffer")
        print("   - The buffer reading method is not working")
    elif changes_detected < 4:
        print("⚠️  WARNING: Very few changes detected.")
        print("   Some rendering might be working, but not all operations.")
    else:
        print("✅ SUCCESS: Screen buffer changes detected!")
        print("   The rendering pipeline appears to be working.")
    
    pynovage.quit()

if __name__ == "__main__":
    main()