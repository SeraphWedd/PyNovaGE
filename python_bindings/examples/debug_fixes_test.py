#!/usr/bin/env python3
"""
Debug Fixes Test
Tests the specific fixes we made to particle system bindings:
1. Vector4 color component access (r, g, b, a)
2. EmitterConfig nested struct access (initial, animation)
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))

import pynovage

def test_vector4_color_components():
    """Test Vector4 color component access (r, g, b, a)"""
    print("🔧 Testing Vector4 color component access...")
    
    try:
        # Create Vector4
        color = pynovage.math.Vector4(1.0, 0.5, 0.25, 0.8)
        
        # Test x, y, z, w access (should work) with floating point tolerance
        assert abs(color.x - 1.0) < 0.001, f"x expected 1.0, got {color.x}"
        assert abs(color.y - 0.5) < 0.001, f"y expected 0.5, got {color.y}"
        assert abs(color.z - 0.25) < 0.001, f"z expected 0.25, got {color.z}"
        assert abs(color.w - 0.8) < 0.001, f"w expected 0.8, got {color.w}"
        print("  ✅ x, y, z, w components work")
        
        # Test r, g, b, a access (this was the issue)
        assert abs(color.r - 1.0) < 0.001, f"r expected 1.0, got {color.r}"  # same as x
        assert abs(color.g - 0.5) < 0.001, f"g expected 0.5, got {color.g}"  # same as y
        assert abs(color.b - 0.25) < 0.001, f"b expected 0.25, got {color.b}" # same as z
        assert abs(color.a - 0.8) < 0.001, f"a expected 0.8, got {color.a}"  # same as w
        print("  ✅ r, g, b, a color components work")
        
        # Test s, t, p, q access (texture coordinates)
        assert abs(color.s - 1.0) < 0.001, f"s expected 1.0, got {color.s}"  # same as x
        assert abs(color.t - 0.5) < 0.001, f"t expected 0.5, got {color.t}"  # same as y
        assert abs(color.p - 0.25) < 0.001, f"p expected 0.25, got {color.p}" # same as z
        assert abs(color.q - 0.8) < 0.001, f"q expected 0.8, got {color.q}"  # same as w
        print("  ✅ s, t, p, q texture components work")
        
        # Test modification
        color.r = 0.9
        assert abs(color.x - 0.9) < 0.001, f"Modified x expected 0.9, got {color.x}"
        print("  ✅ Color component modification works")
        
        return True
        
    except Exception as e:
        print(f"  ❌ Vector4 color component test failed: {e}")
        return False

def test_emitter_config_nested_structs():
    """Test EmitterConfig nested struct access (initial, animation)"""
    print("\n🔧 Testing EmitterConfig nested struct access...")
    
    try:
        # Create EmitterConfig
        config = pynovage.particles.EmitterConfig()
        print("  ✅ EmitterConfig created")
        
        # Test access to initial struct
        initial = config.initial
        print(f"  ✅ initial struct accessed: {type(initial)}")
        
        # Test access to animation struct
        animation = config.animation
        print(f"  ✅ animation struct accessed: {type(animation)}")
        
        # Test initial struct properties
        initial.position_min = pynovage.math.Vector2(0.0, 0.0)
        initial.position_max = pynovage.math.Vector2(10.0, 10.0)
        initial.velocity_min = pynovage.math.Vector2(-5.0, -5.0)
        initial.velocity_max = pynovage.math.Vector2(5.0, 5.0)
        initial.rotation_min = 0.0
        initial.rotation_max = 360.0
        initial.size_min = pynovage.math.Vector2(1.0, 1.0)
        initial.size_max = pynovage.math.Vector2(2.0, 2.0)
        initial.color_min = pynovage.math.Vector4(1.0, 1.0, 1.0, 1.0)
        initial.color_max = pynovage.math.Vector4(0.5, 0.5, 0.5, 0.5)
        initial.lifetime_min = 1.0
        initial.lifetime_max = 3.0
        print("  ✅ initial struct properties set successfully")
        
        # Test animation struct properties
        animation.size_start = 1.0
        animation.size_end = 0.1
        animation.color_start = pynovage.math.Vector4(1.0, 1.0, 1.0, 1.0)
        animation.color_end = pynovage.math.Vector4(1.0, 0.0, 0.0, 0.0)
        print("  ✅ animation struct properties set successfully")
        
        # Verify values were set with floating point tolerance
        assert abs(initial.position_max.x - 10.0) < 0.001, f"position_max.x expected 10.0, got {initial.position_max.x}"
        assert abs(initial.velocity_min.x - (-5.0)) < 0.001, f"velocity_min.x expected -5.0, got {initial.velocity_min.x}"
        assert abs(initial.rotation_max - 360.0) < 0.001, f"rotation_max expected 360.0, got {initial.rotation_max}"
        assert abs(initial.color_max.r - 0.5) < 0.001, f"color_max.r expected 0.5, got {initial.color_max.r}"
        assert abs(animation.size_end - 0.1) < 0.001, f"size_end expected 0.1, got {animation.size_end}"
        assert abs(animation.color_end.r - 1.0) < 0.001, f"color_end.r expected 1.0, got {animation.color_end.r}"
        print("  ✅ nested struct values verified")
        
        return True
        
    except Exception as e:
        print(f"  ❌ EmitterConfig nested struct test failed: {e}")
        return False

def test_particle_init_data():
    """Test ParticleInitData with Vector4 colors"""
    print("\n🔧 Testing ParticleInitData with Vector4 colors...")
    
    try:
        # Create ParticleInitData
        init_data = pynovage.particles.ParticleInitData()
        print("  ✅ ParticleInitData created")
        
        # Set color using both x,y,z,w and r,g,b,a
        init_data.color = pynovage.math.Vector4(1.0, 0.5, 0.25, 0.8)
        
        # Test access via different component names with floating point tolerance
        color = init_data.color
        assert abs(color.x - 1.0) < 0.001, f"x expected 1.0, got {color.x}"
        assert abs(color.r - 1.0) < 0.001, f"r expected 1.0, got {color.r}"
        assert abs(color.g - 0.5) < 0.001, f"g expected 0.5, got {color.g}"
        assert abs(color.b - 0.25) < 0.001, f"b expected 0.25, got {color.b}"
        assert abs(color.a - 0.8) < 0.001, f"a expected 0.8, got {color.a}"
        print("  ✅ ParticleInitData color components work")
        
        return True
        
    except Exception as e:
        print(f"  ❌ ParticleInitData test failed: {e}")
        return False

def test_comprehensive_particle_emitter():
    """Test complete particle emitter with all fixed components"""
    print("\n🔧 Testing comprehensive particle emitter...")
    
    try:
        # Create emitter config with nested structs
        config = pynovage.particles.EmitterConfig()
        config.emission_rate = 50.0
        config.auto_emit = True
        config.looping = True
        config.duration = 10.0
        
        # Configure initial properties (nested struct)
        config.initial.position_min = pynovage.math.Vector2(-5.0, -5.0)
        config.initial.position_max = pynovage.math.Vector2(5.0, 5.0)
        config.initial.velocity_min = pynovage.math.Vector2(-10.0, -10.0)
        config.initial.velocity_max = pynovage.math.Vector2(10.0, 10.0)
        config.initial.color_min = pynovage.math.Vector4(1.0, 0.8, 0.6, 1.0)
        config.initial.color_max = pynovage.math.Vector4(1.0, 1.0, 1.0, 1.0)
        config.initial.lifetime_min = 2.0
        config.initial.lifetime_max = 5.0
        
        # Configure animation properties (nested struct)
        config.animation.size_start = 2.0
        config.animation.size_end = 0.1
        config.animation.color_start = pynovage.math.Vector4(1.0, 1.0, 1.0, 1.0)
        config.animation.color_end = pynovage.math.Vector4(1.0, 0.2, 0.0, 0.0)
        
        print("  ✅ EmitterConfig fully configured with nested structs")
        
        # Create emitter
        emitter = pynovage.particles.ParticleEmitter(config)
        print("  ✅ ParticleEmitter created with configured settings")
        
        # Test emitter methods
        emitter.start()
        assert emitter.is_active() == True
        print("  ✅ Emitter started and is active")
        
        # Test color access in config with floating point tolerance
        start_color = config.animation.color_start
        end_color = config.animation.color_end
        assert abs(start_color.r - 1.0) < 0.001, f"start_color.r expected 1.0, got {start_color.r}"
        assert abs(start_color.g - 1.0) < 0.001, f"start_color.g expected 1.0, got {start_color.g}"
        assert abs(start_color.b - 1.0) < 0.001, f"start_color.b expected 1.0, got {start_color.b}"
        assert abs(start_color.a - 1.0) < 0.001, f"start_color.a expected 1.0, got {start_color.a}"
        assert abs(end_color.r - 1.0) < 0.001, f"end_color.r expected 1.0, got {end_color.r}"
        assert abs(end_color.g - 0.2) < 0.001, f"end_color.g expected 0.2, got {end_color.g}"
        assert abs(end_color.b - 0.0) < 0.001, f"end_color.b expected 0.0, got {end_color.b}"
        assert abs(end_color.a - 0.0) < 0.001, f"end_color.a expected 0.0, got {end_color.a}"
        print("  ✅ Color component access in nested structs works")
        
        return True
        
    except Exception as e:
        print(f"  ❌ Comprehensive particle emitter test failed: {e}")
        return False

def main():
    print("🚀 DEBUG FIXES VERIFICATION TEST")
    print("=" * 50)
    
    tests = [
        ("Vector4 Color Components", test_vector4_color_components),
        ("EmitterConfig Nested Structs", test_emitter_config_nested_structs),
        ("ParticleInitData", test_particle_init_data),
        ("Comprehensive Emitter", test_comprehensive_particle_emitter)
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        try:
            if test_func():
                passed += 1
                print(f"✅ {test_name} - PASSED")
            else:
                print(f"❌ {test_name} - FAILED")
        except Exception as e:
            print(f"❌ {test_name} - FAILED: {e}")
    
    print("\n" + "=" * 50)
    print(f"📊 TEST RESULTS: {passed}/{total} tests passed")
    
    if passed == total:
        print("🎉 ALL DEBUG FIXES VERIFIED! The particle system is now fully functional.")
        return True
    else:
        print("⚠️  Some fixes still need work.")
        return False

if __name__ == "__main__":
    main()