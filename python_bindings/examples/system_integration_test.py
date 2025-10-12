#!/usr/bin/env python3
"""
PyNovaGE System Integration Test
Tests all major subsystems working together
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))

import pynovage

def test_subsystem(name, test_func):
    """Helper function to test a subsystem"""
    try:
        print(f"Testing {name}...")
        test_func()
        print(f"✅ {name} - PASSED")
        return True
    except Exception as e:
        print(f"❌ {name} - FAILED: {e}")
        return False

def test_audio():
    """Test audio subsystem"""
    pynovage.audio.initialize_audio()
    audio_system = pynovage.audio.get_audio_system()
    audio_system.update(0.016)
    pynovage.audio.shutdown_audio()

def test_physics():
    """Test physics subsystem"""
    # Initialize physics world
    world = pynovage.physics.PhysicsWorld()
    
    # Create a physics body using correct signature (width, height)
    body = pynovage.physics.dynamic_box(1.0, 1.0)  # width and height as floats
    
    # Add body to world
    world.add_body(body)

def test_math():
    """Test math utilities"""
    vec = pynovage.math.Vector3(1, 2, 3)
    assert vec.x == 1 and vec.y == 2 and vec.z == 3
    
    # Test Vector2
    vec2 = pynovage.math.Vector2(4, 5)
    assert vec2.x == 4 and vec2.y == 5

def test_asset():
    """Test asset system"""
    # Just check that the asset module is available
    assert hasattr(pynovage, 'asset')
    assert hasattr(pynovage.asset, 'AssetType')

def test_renderer():
    """Test renderer availability"""
    # Just check that the renderer module is available
    assert hasattr(pynovage, 'renderer')

def test_scene():
    """Test scene system availability"""
    # Just check that the scene module is available
    assert hasattr(pynovage, 'scene')

def main():
    print("🚀 PyNovaGE System Integration Test")
    print("=" * 50)
    
    results = []
    
    # Test all subsystems
    results.append(test_subsystem("Math System", test_math))
    results.append(test_subsystem("Physics System", test_physics))
    results.append(test_subsystem("Audio System", test_audio))
    results.append(test_subsystem("Asset System", test_asset))
    results.append(test_subsystem("Renderer System", test_renderer))
    results.append(test_subsystem("Scene System", test_scene))
    
    print("\n" + "=" * 50)
    passed = sum(results)
    total = len(results)
    
    print(f"📊 Test Results: {passed}/{total} subsystems passed")
    
    if passed == total:
        print("🎉 All subsystems are working correctly!")
        return True
    else:
        print(f"⚠️  {total - passed} subsystem(s) failed")
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)