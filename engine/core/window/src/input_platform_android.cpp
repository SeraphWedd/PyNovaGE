#ifdef PYNOVAGE_PLATFORM_ANDROID

#include "window/input_platform.hpp"
#include "window/input.hpp"
#include <android/input.h>
#include <android/keycodes.h>
#include <android/native_activity.h>
#include <android/looper.h>
#include <unordered_map>
#include <algorithm>

namespace PyNovaGE {
namespace Window {

class AndroidInputPlatform::AndroidInputImpl {
public:
    explicit AndroidInputImpl(ANativeActivity* activity) 
        : activity_(activity) {
        
        // Initialize touch data tracking
        active_touches_.reserve(10); // Support up to 10 touches
        
        // Initialize key mapping
        InitializeKeyMapping();
    }
    
    bool Initialize() {
        if (!activity_) {
            return false;
        }
        
        // Set up input event handling
        AInputQueue* input_queue = nullptr;
        // Note: In a full implementation, you'd get the input queue from the native activity
        // and set up the input handling loop
        
        return true;
    }
    
    void Shutdown() {
        active_touches_.clear();
        gamepad_states_.clear();
    }
    
    void Update() {
        // Process pending input events from Android's input queue
        ProcessPendingEvents();
        
        // Update gamepad states
        UpdateGamepads();
        
        // Clean up inactive touches
        active_touches_.erase(
            std::remove_if(active_touches_.begin(), active_touches_.end(),
                          [](const TouchData& touch) { return !touch.active; }),
            active_touches_.end()
        );
    }
    
    // Touch input methods
    int GetActiveTouchCount() const {
        return static_cast<int>(active_touches_.size());
    }
    
    TouchData GetTouch(int touch_id) const {
        auto it = std::find_if(active_touches_.begin(), active_touches_.end(),
                              [touch_id](const TouchData& touch) { return touch.id == touch_id; });
        return (it != active_touches_.end()) ? *it : TouchData{};
    }
    
    std::vector<TouchData> GetActiveTouches() const {
        return active_touches_;
    }
    
    // Key input methods
    bool IsKeyPressed(Key key) const {
        auto it = key_states_.find(key);
        return (it != key_states_.end()) && (it->second == InputState::Pressed || it->second == InputState::Held);
    }
    
    bool IsKeyJustPressed(Key key) const {
        auto it = key_states_.find(key);
        return (it != key_states_.end()) && (it->second == InputState::Pressed);
    }
    
    bool IsKeyJustReleased(Key key) const {
        auto it = key_states_.find(key);
        auto prev_it = prev_key_states_.find(key);
        return (it == key_states_.end() || it->second == InputState::Released) &&
               (prev_it != prev_key_states_.end() && (prev_it->second == InputState::Pressed || prev_it->second == InputState::Held));
    }
    
    // Gamepad methods (Android supports game controllers)
    bool IsGamepadConnected(int gamepad_id) const {
        auto it = gamepad_states_.find(gamepad_id);
        return (it != gamepad_states_.end()) && it->second.connected;
    }
    
    const GamepadState& GetGamepadState(int gamepad_id) const {
        auto it = gamepad_states_.find(gamepad_id);
        return (it != gamepad_states_.end()) ? it->second : empty_gamepad_state_;
    }
    
    // Event callbacks
    InputCallback input_callback_;
    TouchCallback touch_callback_;

private:
    ANativeActivity* activity_;
    
    // Touch tracking
    std::vector<TouchData> active_touches_;
    
    // Key state tracking
    std::unordered_map<Key, InputState> key_states_;
    std::unordered_map<Key, InputState> prev_key_states_;
    
    // Gamepad state tracking
    std::unordered_map<int, GamepadState> gamepad_states_;
    std::unordered_map<int, GamepadState> prev_gamepad_states_;
    
    // Android keycode to engine Key mapping
    std::unordered_map<int32_t, Key> android_key_mapping_;
    
    // Empty gamepad state for invalid IDs
    GamepadState empty_gamepad_state_{};
    
    void InitializeKeyMapping() {
        // Map Android keycodes to our engine Key enum
        android_key_mapping_[AKEYCODE_A] = Key::A;
        android_key_mapping_[AKEYCODE_B] = Key::B;
        android_key_mapping_[AKEYCODE_C] = Key::C;
        android_key_mapping_[AKEYCODE_D] = Key::D;
        android_key_mapping_[AKEYCODE_E] = Key::E;
        android_key_mapping_[AKEYCODE_F] = Key::F;
        android_key_mapping_[AKEYCODE_G] = Key::G;
        android_key_mapping_[AKEYCODE_H] = Key::H;
        android_key_mapping_[AKEYCODE_I] = Key::I;
        android_key_mapping_[AKEYCODE_J] = Key::J;
        android_key_mapping_[AKEYCODE_K] = Key::K;
        android_key_mapping_[AKEYCODE_L] = Key::L;
        android_key_mapping_[AKEYCODE_M] = Key::M;
        android_key_mapping_[AKEYCODE_N] = Key::N;
        android_key_mapping_[AKEYCODE_O] = Key::O;
        android_key_mapping_[AKEYCODE_P] = Key::P;
        android_key_mapping_[AKEYCODE_Q] = Key::Q;
        android_key_mapping_[AKEYCODE_R] = Key::R;
        android_key_mapping_[AKEYCODE_S] = Key::S;
        android_key_mapping_[AKEYCODE_T] = Key::T;
        android_key_mapping_[AKEYCODE_U] = Key::U;
        android_key_mapping_[AKEYCODE_V] = Key::V;
        android_key_mapping_[AKEYCODE_W] = Key::W;
        android_key_mapping_[AKEYCODE_X] = Key::X;
        android_key_mapping_[AKEYCODE_Y] = Key::Y;
        android_key_mapping_[AKEYCODE_Z] = Key::Z;
        
        android_key_mapping_[AKEYCODE_0] = Key::Num0;
        android_key_mapping_[AKEYCODE_1] = Key::Num1;
        android_key_mapping_[AKEYCODE_2] = Key::Num2;
        android_key_mapping_[AKEYCODE_3] = Key::Num3;
        android_key_mapping_[AKEYCODE_4] = Key::Num4;
        android_key_mapping_[AKEYCODE_5] = Key::Num5;
        android_key_mapping_[AKEYCODE_6] = Key::Num6;
        android_key_mapping_[AKEYCODE_7] = Key::Num7;
        android_key_mapping_[AKEYCODE_8] = Key::Num8;
        android_key_mapping_[AKEYCODE_9] = Key::Num9;
        
        android_key_mapping_[AKEYCODE_SPACE] = Key::Space;
        android_key_mapping_[AKEYCODE_ENTER] = Key::Enter;
        android_key_mapping_[AKEYCODE_DEL] = Key::Backspace;
        android_key_mapping_[AKEYCODE_TAB] = Key::Tab;
        android_key_mapping_[AKEYCODE_ESCAPE] = Key::Escape;
        android_key_mapping_[AKEYCODE_DPAD_UP] = Key::Up;
        android_key_mapping_[AKEYCODE_DPAD_DOWN] = Key::Down;
        android_key_mapping_[AKEYCODE_DPAD_LEFT] = Key::Left;
        android_key_mapping_[AKEYCODE_DPAD_RIGHT] = Key::Right;
        android_key_mapping_[AKEYCODE_BACK] = Key::Escape; // Android back button -> Escape
        
        // Add more mappings as needed...
    }
    
    void ProcessPendingEvents() {
        // This is where you'd process events from Android's input queue
        // In a full implementation, you'd:
        // 1. Get events from AInputQueue
        // 2. Process touch events (AMOTION_EVENT_ACTION_DOWN, UP, MOVE, etc.)
        // 3. Process key events (AKEY_EVENT_ACTION_DOWN, UP)
        // 4. Convert Android events to our engine events
        // 5. Trigger callbacks
        
        // Example touch event processing (pseudo-code):
        /*
        AInputEvent* event = nullptr;
        while (AInputQueue_getEvent(input_queue, &event) >= 0) {
            int32_t event_type = AInputEvent_getType(event);
            
            if (event_type == AINPUT_EVENT_TYPE_MOTION) {
                int32_t action = AMotionEvent_getAction(event);
                int32_t pointer_index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                int32_t pointer_id = AMotionEvent_getPointerId(event, pointer_index);
                float x = AMotionEvent_getX(event, pointer_index);
                float y = AMotionEvent_getY(event, pointer_index);
                float pressure = AMotionEvent_getPressure(event, pointer_index);
                
                TouchData touch;
                touch.id = pointer_id;
                touch.position = {x, y};
                touch.pressure = pressure;
                
                switch (action & AMOTION_EVENT_ACTION_MASK) {
                    case AMOTION_EVENT_ACTION_DOWN:
                    case AMOTION_EVENT_ACTION_POINTER_DOWN:
                        touch.active = true;
                        active_touches_.push_back(touch);
                        if (touch_callback_) {
                            TouchEvent touch_event;
                            touch_event.type = TouchEventType::TouchDown;
                            touch_event.touch = touch;
                            touch_callback_(touch_event);
                        }
                        break;
                        
                    case AMOTION_EVENT_ACTION_UP:
                    case AMOTION_EVENT_ACTION_POINTER_UP:
                        // Update existing touch and mark as inactive
                        for (auto& existing_touch : active_touches_) {
                            if (existing_touch.id == pointer_id) {
                                existing_touch.active = false;
                                if (touch_callback_) {
                                    TouchEvent touch_event;
                                    touch_event.type = TouchEventType::TouchUp;
                                    touch_event.touch = existing_touch;
                                    touch_callback_(touch_event);
                                }
                                break;
                            }
                        }
                        break;
                        
                    case AMOTION_EVENT_ACTION_MOVE:
                        // Update existing touches
                        for (auto& existing_touch : active_touches_) {
                            if (existing_touch.id == pointer_id) {
                                Vector2f old_pos = existing_touch.position;
                                existing_touch.position = {x, y};
                                existing_touch.pressure = pressure;
                                if (touch_callback_) {
                                    TouchEvent touch_event;
                                    touch_event.type = TouchEventType::TouchMove;
                                    touch_event.touch = existing_touch;
                                    touch_event.delta = existing_touch.position - old_pos;
                                    touch_callback_(touch_event);
                                }
                                break;
                            }
                        }
                        break;
                }
            }
            
            AInputQueue_finishEvent(input_queue, event, 1);
        }
        */
    }
    
    void UpdateGamepads() {
        // Update gamepad states using Android's game controller API
        // Android supports Xbox and PlayStation controllers via USB/Bluetooth
        
        prev_gamepad_states_ = gamepad_states_;
        
        // In a full implementation, you'd enumerate connected input devices
        // and check if they're game controllers
    }
    
    InputState GetInputState(bool current, bool previous) const {
        if (current && previous) {
            return InputState::Held;
        } else if (current && !previous) {
            return InputState::Pressed;
        } else {
            return InputState::Released;
        }
    }
};

// AndroidInputPlatform implementation
AndroidInputPlatform::AndroidInputPlatform(void* native_activity) 
    : impl_(std::make_unique<AndroidInputImpl>(static_cast<ANativeActivity*>(native_activity))) {
}

AndroidInputPlatform::~AndroidInputPlatform() = default;

bool AndroidInputPlatform::Initialize() {
    return impl_->Initialize();
}

void AndroidInputPlatform::Shutdown() {
    impl_->Shutdown();
}

void AndroidInputPlatform::Update() {
    impl_->Update();
}

void AndroidInputPlatform::SetInputCallback(InputCallback callback) {
    impl_->input_callback_ = std::move(callback);
}

void AndroidInputPlatform::SetTouchCallback(TouchCallback callback) {
    impl_->touch_callback_ = std::move(callback);
}

bool AndroidInputPlatform::IsKeyPressed(Key key) const {
    return impl_->IsKeyPressed(key);
}

bool AndroidInputPlatform::IsKeyJustPressed(Key key) const {
    return impl_->IsKeyJustPressed(key);
}

bool AndroidInputPlatform::IsKeyJustReleased(Key key) const {
    return impl_->IsKeyJustReleased(key);
}

int AndroidInputPlatform::GetActiveTouchCount() const {
    return impl_->GetActiveTouchCount();
}

TouchData AndroidInputPlatform::GetTouch(int touch_id) const {
    return impl_->GetTouch(touch_id);
}

std::vector<TouchData> AndroidInputPlatform::GetActiveTouches() const {
    return impl_->GetActiveTouches();
}

bool AndroidInputPlatform::IsGamepadConnected(int gamepad_id) const {
    return impl_->IsGamepadConnected(gamepad_id);
}

const GamepadState& AndroidInputPlatform::GetGamepadState(int gamepad_id) const {
    return impl_->GetGamepadState(gamepad_id);
}

bool AndroidInputPlatform::IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const {
    (void)gamepad_id; (void)button;
    return false; // TODO: Implement
}

bool AndroidInputPlatform::IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
    (void)gamepad_id; (void)button;
    return false; // TODO: Implement
}

bool AndroidInputPlatform::IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
    (void)gamepad_id; (void)button;
    return false; // TODO: Implement
}

float AndroidInputPlatform::GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
    (void)gamepad_id; (void)axis;
    return 0.0f; // TODO: Implement
}

} // namespace Window  
} // namespace PyNovaGE

#endif // PYNOVAGE_PLATFORM_ANDROID