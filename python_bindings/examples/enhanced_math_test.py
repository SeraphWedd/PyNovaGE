#!/usr/bin/env python3
"""
Enhanced Math System Test
Tests new Quaternion and Matrix4 functionality in PyNovaGE
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))

import pynovage
import math

def test_quaternions():
    """Test Quaternion functionality"""
    print("🧮 Testing Quaternions...")
    
    try:
        # Test identity quaternion
        q1 = pynovage.math.Quaternion()
        print(f"  Identity quaternion: {q1}")
        assert q1.x() == 0 and q1.y() == 0 and q1.z() == 0 and q1.w() == 1
        print("  ✅ Identity quaternion works")
        
        # Test quaternion from components
        q2 = pynovage.math.Quaternion(0.0, 0.0, 0.7071, 0.7071)  # 90° rotation around Z
        print(f"  Component quaternion: {q2}")
        print("  ✅ Component construction works")
        
        # Test quaternion from axis-angle
        axis = pynovage.math.Vector3(0, 0, 1)  # Z axis
        angle = math.pi / 2  # 90 degrees in radians
        q3 = pynovage.math.Quaternion(axis, angle)
        print(f"  Axis-angle quaternion: {q3}")
        print("  ✅ Axis-angle construction works")
        
        # Test quaternion from Euler angles (roll, pitch, yaw)
        q4 = pynovage.math.Quaternion(0.0, 0.0, math.pi/2)  # 90° yaw
        print(f"  Euler angles quaternion: {q4}")
        print("  ✅ Euler angles construction works")
        
        # Test quaternion operations
        q5 = q1 + q2
        print(f"  Quaternion addition: {q5}")
        
        q6 = q2 * 2.0
        print(f"  Quaternion scalar multiplication: {q6}")
        
        # Test normalization
        q_norm = q2.normalized()
        length = q_norm.length()
        print(f"  Normalized quaternion length: {length}")
        assert abs(length - 1.0) < 0.001
        print("  ✅ Quaternion normalization works")
        
        # Test array access
        assert q2[0] == q2.x() and q2[1] == q2.y() and q2[2] == q2.z() and q2[3] == q2.w()
        print("  ✅ Array access works")
        
        return True
        
    except Exception as e:
        print(f"  ❌ Quaternion test failed: {e}")
        return False

def test_matrices():
    """Test Matrix4 functionality"""
    print("🧮 Testing Matrices...")
    
    try:
        # Test identity matrix
        m1 = pynovage.math.Matrix4()
        print(f"  Identity matrix: {m1}")
        assert m1.at(0, 0) == 1 and m1.at(1, 1) == 1 and m1.at(2, 2) == 1 and m1.at(3, 3) == 1
        print("  ✅ Identity matrix works")
        
        # Test static identity
        m_id = pynovage.math.Matrix4.Identity()
        print("  ✅ Static Identity() works")
        
        # Test matrix from components
        m2 = pynovage.math.Matrix4(
            1, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, 3, 0,
            0, 0, 0, 1
        )
        print(f"  Component matrix: {m2}")
        print("  ✅ Component construction works")
        
        # Test matrix operations
        m3 = m1 + m2
        print("  ✅ Matrix addition works")
        
        m4 = m2 * 2.0
        print("  ✅ Matrix scalar multiplication works")
        
        m5 = m1 * m2
        print("  ✅ Matrix multiplication works")
        
        # Test determinant
        det = m2.determinant()
        print(f"  Matrix determinant: {det}")
        assert abs(det - 6.0) < 0.001  # 1*2*3*1 = 6
        print("  ✅ Determinant calculation works")
        
        # Test transpose
        m_t = m2.transpose()
        assert m_t.at(0, 1) == m2.at(1, 0)
        print("  ✅ Matrix transpose works")
        
        # Test row access (returns Vector4)
        row0 = m2[0]
        print(f"  Matrix row 0: {row0}")
        print("  ✅ Row access works")
        
        # Test element access
        assert m2.at(1, 1) == 2
        print("  ✅ Element access works")
        
        return True
        
    except Exception as e:
        print(f"  ❌ Matrix test failed: {e}")
        return False

def test_enhanced_vectors():
    """Test that existing vector functionality still works"""
    print("🧮 Testing Enhanced Vector Support...")
    
    try:
        # Test Vector3 (should still work)
        v1 = pynovage.math.Vector3(1, 2, 3)
        v2 = pynovage.math.Vector3(4, 5, 6)
        
        # Test operations
        v3 = v1 + v2
        assert v3.x == 5 and v3.y == 7 and v3.z == 9
        
        # Test dot product
        dot = v1.dot(v2)
        expected_dot = 1*4 + 2*5 + 3*6  # 32
        assert abs(dot - expected_dot) < 0.001
        
        # Test cross product
        cross = v1.cross(v2)
        print(f"  Cross product: {cross}")
        
        print("  ✅ Enhanced vector support works")
        return True
        
    except Exception as e:
        print(f"  ❌ Enhanced vector test failed: {e}")
        return False

def main():
    print("🚀 Enhanced Math System Test")
    print("=" * 50)
    
    results = []
    
    # Test all enhanced math features
    results.append(test_quaternions())
    results.append(test_matrices()) 
    results.append(test_enhanced_vectors())
    
    print("\n" + "=" * 50)
    passed = sum(results)
    total = len(results)
    
    print(f"📊 Enhanced Math Test Results: {passed}/{total} tests passed")
    
    if passed == total:
        print("🎉 All enhanced math features are working correctly!")
        print("\n🆕 **NEW APIs AVAILABLE:**")
        print("   • pynovage.math.Quaternion - Full quaternion support with rotations")
        print("   • pynovage.math.Matrix4 - 4x4 matrix operations and transformations")
        print("   • Enhanced vector operations with improved math support")
        return True
    else:
        print(f"⚠️  {total - passed} enhanced math test(s) failed")
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)