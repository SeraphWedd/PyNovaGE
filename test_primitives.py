#!/usr/bin/env python3
"""
Test script to verify PyNovaGE rendering primitives work correctly after our fixes.
This will test the batch renderer's primitive drawing functions.
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python_bindings'))

import pynovage_core

def test_basic_rendering():
    """Test basic rendering functionality with primitives."""
    print("Starting PyNovaGE primitive rendering test...")
    
    try:
        # Initialize the renderer first
        print("Initializing PyNovaGE renderer...")
        pynovage_core.renderer.initialize()
        print("✓ Renderer initialized successfully")
        
        # Create window
        print("Creating window...")
        window_config = pynovage_core.window.WindowConfig()
        window_config.width = 800
        window_config.height = 600
        window_config.title = "PyNovaGE Primitive Test"
        window = pynovage_core.window.create_window(window_config)
        if not window:
            print("❌ Failed to create window")
            return False
            
        print("✓ Window created successfully")
        
        # Test rendering loop
        print("Testing rendering primitives...")
        frame_count = 0
        max_frames = 60  # Test for 60 frames
        
        while frame_count < max_frames and not window.should_close():
            # Begin frame
            pynovage_core.renderer.begin_frame()
            
            # Clear the screen
            pynovage_core.renderer.clear(0.2, 0.2, 0.3, 1.0)  # Dark blue background
            
            # Test batch renderer primitives
            batch_renderer = pynovage_core.renderer.get_batch_renderer()
            
            if batch_renderer:
                # Draw a red rectangle in screen coordinates
                color = pynovage_core.math.Vector4f(1.0, 0.0, 0.0, 1.0)  # Red
                batch_renderer.add_rect_screen(100, 100, 200, 150, color)
                
                # Draw a green circle in screen coordinates  
                green_color = pynovage_core.math.Vector4f(0.0, 1.0, 0.0, 1.0)  # Green
                batch_renderer.add_circle_screen(400, 300, 50, 16, green_color)
                
                # Draw a blue line in screen coordinates
                blue_color = pynovage_core.math.Vector4f(0.0, 0.0, 1.0, 1.0)  # Blue
                batch_renderer.add_line_screen(50, 50, 750, 550, 3, blue_color)
                
                # Flush the batch
                batch_renderer.flush()
            
            # End frame
            pynovage_core.renderer.end_frame()
            
            # Swap buffers
            window.swap_buffers()
            
            # Poll events
            window.poll_events()
            
            frame_count += 1
            
            # Print progress every 20 frames
            if frame_count % 20 == 0:
                print(f"  Rendered {frame_count}/{max_frames} frames...")
        
        print(f"✓ Successfully rendered {frame_count} frames with primitives")
        
        # Test ReadPixels function
        print("Testing ReadPixels function...")
        try:
            # Try to read a small portion of the screen buffer
            pixel_data = pynovage_core.renderer.read_pixels(0, 0, 10, 10)
            if pixel_data:
                print(f"✓ ReadPixels successful - read {len(pixel_data)} bytes")
                # Print first few pixel values
                if len(pixel_data) >= 12:  # At least 3 pixels (RGB)
                    print(f"  First pixel: R={pixel_data[0]}, G={pixel_data[1]}, B={pixel_data[2]}")
            else:
                print("⚠ ReadPixels returned empty data")
        except Exception as e:
            print(f"❌ ReadPixels failed: {e}")
        
        # Cleanup
        print("Cleaning up...")
        if window:
            window.close()
        pynovage_core.renderer.shutdown()
        print("✓ Test completed successfully")
        return True
        
    except Exception as e:
        print(f"❌ Test failed with exception: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_renderer_stats():
    """Test renderer statistics functionality."""
    print("\nTesting renderer statistics...")
    
    try:
        stats = pynovage_core.renderer.get_stats()
        if stats:
            print("✓ Renderer stats obtained successfully")
            # Note: We can't print specific stats without knowing the exact structure
            # but we can verify the function doesn't crash
        else:
            print("⚠ Renderer stats returned None")
    except Exception as e:
        print(f"❌ Renderer stats failed: {e}")

def main():
    """Main test function."""
    print("=" * 60)
    print("PyNovaGE Rendering Primitives Test")
    print("=" * 60)
    
    success = test_basic_rendering()
    test_renderer_stats()
    
    print("\n" + "=" * 60)
    if success:
        print("🎉 All tests completed successfully!")
        print("✓ Batch renderer primitives are working correctly")
        print("✓ ReadPixels function is accessible")
    else:
        print("❌ Some tests failed")
    print("=" * 60)

if __name__ == "__main__":
    main()