#!/usr/bin/env python3
"""
Simple test to determine correct PyNovaGE API usage.
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python_bindings'))

import pynovage_core

def test_simple_init():
    """Test simple initialization."""
    try:
        print("Testing simple initialization...")
        
        # Initialize renderer
        pynovage_core.renderer.initialize()
        print("✓ Renderer initialized")
        
        # Check if batch renderer is available
        batch_renderer = pynovage_core.renderer.get_batch_renderer()
        print(f"✓ Batch renderer obtained: {type(batch_renderer)}")
        
        # Test ReadPixels function
        print("Testing ReadPixels function...")
        try:
            pixel_data = pynovage_core.renderer.read_pixels(0, 0, 1, 1)
            print(f"✓ ReadPixels function accessible, returned: {type(pixel_data)}")
            if pixel_data:
                print(f"  Data length: {len(pixel_data)}")
            else:
                print("  Data is None or empty")
        except Exception as e:
            print(f"❌ ReadPixels failed: {e}")
        
        # Test renderer stats
        print("Testing renderer stats...")
        try:
            stats = pynovage_core.renderer.get_stats()
            print(f"✓ Renderer stats obtained: {type(stats)}")
        except Exception as e:
            print(f"❌ Renderer stats failed: {e}")
        
        # Cleanup
        pynovage_core.renderer.shutdown()
        print("✓ All tests completed")
        
    except Exception as e:
        print(f"❌ Test failed: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_simple_init()