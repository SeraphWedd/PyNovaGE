#!/usr/bin/env python3
"""
Audio System Test
Tests basic audio initialization and functionality
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))

import pynovage

def main():
    print("Testing PyNovaGE Audio System...")
    
    # Check if audio is supported
    print(f"Audio support detected: {pynovage.audio.is_supported()}")
    
    # Initialize audio system
    try:
        print("Initializing audio...")
        pynovage.audio.initialize_audio()
        print("✅ Audio system initialized successfully!")
        
        # Get audio system instance
        audio_system = pynovage.audio.get_audio_system()
        print(f"✅ Got audio system instance: {audio_system}")
        
        # Test audio system methods
        print("Testing audio system functionality...")
        
        # Update audio system (should not crash)
        audio_system.update(0.016)  # 60 FPS delta time
        print("✅ Audio system update completed")
        
        # Test AudioState enum
        print(f"AudioState enum available: {pynovage.audio.AudioState}")
        
        # Test AudioSource class
        print(f"AudioSource class available: {pynovage.audio.AudioSource}")
        
        # Shutdown audio
        print("Shutting down audio...")
        pynovage.audio.shutdown_audio()
        print("✅ Audio system shut down successfully!")
        
    except Exception as e:
        print(f"❌ Audio test failed: {e}")
        return False
    
    print("\n🎉 All audio tests passed!")
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)