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
        // Note: In a real Android app, the input queue would be provided by the framework
        // through ANativeActivity_onCreate or similar callbacks. The queue is typically
        // set up by the Android system and passed to the native activity.
        
        // For a complete implementation, you would:
        // 1. Wait for the input queue to be available (usually set in app callbacks)
        // 2. Set up looper attachment for the input queue
        // 3. Configure input queue callbacks
        
        // Pseudo-implementation (real implementation would be in app lifecycle callbacks):
        /*
        if (activity_->inputQueue) {
            input_queue_ = activity_->inputQueue;
            
            // Attach input queue to looper for processing
            ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
            AInputQueue_attachLooper(input_queue_, looper, ALOOPER_POLL_CALLBACK, nullptr, nullptr);
        }
        */
        
        // For now, mark as initialized (actual queue setup happens in app lifecycle)
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
    
    bool IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const {
        auto it = gamepad_states_.find(gamepad_id);
        if (it == gamepad_states_.end() || !it->second.connected) {
            return false;
        }
        
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= static_cast<int>(it->second.buttons.size())) {
            return false;
        }
        
        return it->second.buttons[btn];
    }
    
    bool IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
        auto it = gamepad_states_.find(gamepad_id);
        auto prev_it = prev_gamepad_states_.find(gamepad_id);
        
        if (it == gamepad_states_.end() || !it->second.connected) {
            return false;
        }
        
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= static_cast<int>(it->second.buttons.size())) {
            return false;
        }
        
        bool current = it->second.buttons[btn];
        bool previous = (prev_it != prev_gamepad_states_.end() && 
                        btn < static_cast<int>(prev_it->second.buttons.size())) 
                       ? prev_it->second.buttons[btn] : false;
        
        return current && !previous;
    }
    
    bool IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
        auto it = gamepad_states_.find(gamepad_id);
        auto prev_it = prev_gamepad_states_.find(gamepad_id);
        
        if (it == gamepad_states_.end()) {
            return false;
        }
        
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= static_cast<int>(it->second.buttons.size())) {
            return false;
        }
        
        bool current = it->second.connected ? it->second.buttons[btn] : false;
        bool previous = (prev_it != prev_gamepad_states_.end() && 
                        prev_it->second.connected &&
                        btn < static_cast<int>(prev_it->second.buttons.size())) 
                       ? prev_it->second.buttons[btn] : false;
        
        return !current && previous;
    }
    
    float GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
        auto it = gamepad_states_.find(gamepad_id);
        if (it == gamepad_states_.end() || !it->second.connected) {
            return 0.0f;
        }
        
        int ax = static_cast<int>(axis);
        if (ax < 0 || ax >= static_cast<int>(it->second.axes.size())) {
            return 0.0f;
        }
        
        return it->second.axes[ax];
    }
    
    // Event callbacks
    InputCallback input_callback_;
    TouchCallback touch_callback_;
    
    // Set input queue (called by app lifecycle)
    void SetInputQueue(AInputQueue* input_queue) {
        input_queue_ = input_queue;
        
        if (input_queue_) {
            // Attach to main looper if available
            ALooper* looper = ALooper_forThread();
            if (looper) {
                AInputQueue_attachLooper(input_queue_, looper, ALOOPER_POLL_CALLBACK, nullptr, nullptr);
            }
        }
    }

private:
    ANativeActivity* activity_;
    AInputQueue* input_queue_ = nullptr;
    
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
        // Process events from Android's input queue
        if (!input_queue_) {
            return;
        }
        
        // Store previous key states
        prev_key_states_ = key_states_;
        
        AInputEvent* event = nullptr;
        while (AInputQueue_getEvent(input_queue_, &event) >= 0) {
            int32_t event_type = AInputEvent_getType(event);
            
            if (event_type == AINPUT_EVENT_TYPE_MOTION) {
                ProcessTouchEvent(event);
            } else if (event_type == AINPUT_EVENT_TYPE_KEY) {
                ProcessKeyEvent(event);
            }
            
            AInputQueue_finishEvent(input_queue_, event, 1);
        }
    }
    
    void UpdateGamepads() {
        // Update gamepad states using Android's game controller API
        prev_gamepad_states_ = gamepad_states_;
        
        // Get all input device IDs from Android
        const int32_t max_devices = 16; // Reasonable limit
        int32_t device_ids[max_devices];
        int32_t device_count = AInputManager_getDeviceIds(AInputManager_getInstance(), device_ids, max_devices);
        
        // Clear disconnected controllers
        auto it = gamepad_states_.begin();
        while (it != gamepad_states_.end()) {
            bool found = false;
            for (int32_t i = 0; i < device_count; ++i) {
                if (it->first == device_ids[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                it = gamepad_states_.erase(it);
            } else {
                ++it;
            }
        }
        
        // Check each device to see if it's a game controller
        for (int32_t i = 0; i < device_count; ++i) {
            int32_t device_id = device_ids[i];
            
            // Get device properties
            AInputDevice* device = AInputManager_getInputDevice(AInputManager_getInstance(), device_id);
            if (!device) continue;
            
            // Check if this device has gamepad-like capabilities
            int32_t sources = AInputDevice_getSources(device);
            bool is_gamepad = (sources & AINPUT_SOURCE_GAMEPAD) != 0 || 
                             (sources & AINPUT_SOURCE_JOYSTICK) != 0;
            
            if (is_gamepad) {
                // Update or create gamepad state
                GamepadState& state = gamepad_states_[device_id];
                state.connected = true;
                
                // Get device name
                const char* name = AInputDevice_getName(device);
                if (name) {
                    state.name = name;
                }
                
                // Update button and axis states
                UpdateGamepadState(device, state);
            }
        }
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
    
    void ProcessTouchEvent(AInputEvent* event) {
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
                // Update all touches for move events (Android batches them)
                size_t pointer_count = AMotionEvent_getPointerCount(event);
                for (size_t i = 0; i < pointer_count; ++i) {
                    int32_t move_pointer_id = AMotionEvent_getPointerId(event, i);
                    float move_x = AMotionEvent_getX(event, i);
                    float move_y = AMotionEvent_getY(event, i);
                    float move_pressure = AMotionEvent_getPressure(event, i);
                    
                    for (auto& existing_touch : active_touches_) {
                        if (existing_touch.id == move_pointer_id) {
                            PyNovaGE::Vector2f old_pos = existing_touch.position;
                            existing_touch.position = {move_x, move_y};
                            existing_touch.pressure = move_pressure;
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
                }
                break;
        }
    }
    
    void ProcessKeyEvent(AInputEvent* event) {
        int32_t keycode = AKeyEvent_getKeyCode(event);
        int32_t action = AKeyEvent_getAction(event);
        
        // Convert Android keycode to our Key enum
        auto key_it = android_key_mapping_.find(keycode);
        if (key_it == android_key_mapping_.end()) {
            return; // Unknown key, ignore
        }
        
        Key engine_key = key_it->second;
        InputState new_state = InputState::Released;
        
        if (action == AKEY_EVENT_ACTION_DOWN) {
            // Check if this is a repeat event
            bool is_repeat = (AKeyEvent_getRepeatCount(event) > 0);
            if (!is_repeat) {
                new_state = InputState::Pressed;
            } else {
                new_state = InputState::Held;
            }
        } else if (action == AKEY_EVENT_ACTION_UP) {
            new_state = InputState::Released;
        }
        
        // Update key state
        key_states_[engine_key] = new_state;
        
        // Trigger input event callback
        if (input_callback_ && action != AKEY_EVENT_ACTION_MULTIPLE) { // Don't send repeat events
            InputEvent input_event{};
            input_event.key = engine_key;
            
            // Get modifier states
            int32_t meta_state = AKeyEvent_getMetaState(event);
            input_event.shift = (meta_state & AMETA_SHIFT_ON) != 0;
            input_event.control = (meta_state & AMETA_CTRL_ON) != 0;
            input_event.alt = (meta_state & AMETA_ALT_ON) != 0;
            
            if (action == AKEY_EVENT_ACTION_DOWN) {
                input_event.type = InputEventType::KeyPress;
            } else if (action == AKEY_EVENT_ACTION_UP) {
                input_event.type = InputEventType::KeyRelease;
            }
            
            input_callback_(input_event);
        }
    }
    
    void UpdateGamepadState(AInputDevice* device, GamepadState& state) {
        if (!device) return;
        
        // Initialize button and axis arrays if needed
        if (state.buttons.size() == 0) {
            state.buttons.fill(false);
            state.axes.fill(0.0f);
        }
        
        // Note: Android doesn't provide a direct way to query current button/axis states
        // outside of input events. In a real implementation, you would:
        // 1. Track button states from motion/key events in ProcessPendingEvents
        // 2. Store axis values from motion events 
        // 3. Update the GamepadState here based on cached values
        
        // For now, we maintain the states through event processing
        // The actual button/axis updates happen in ProcessTouchEvent and ProcessKeyEvent
        // when gamepad events are received
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
    return impl_->IsGamepadButtonPressed(gamepad_id, button);
}

bool AndroidInputPlatform::IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
    return impl_->IsGamepadButtonJustPressed(gamepad_id, button);
}

bool AndroidInputPlatform::IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
    return impl_->IsGamepadButtonJustReleased(gamepad_id, button);
}

float AndroidInputPlatform::GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
    return impl_->GetGamepadAxis(gamepad_id, axis);
}

// Android-specific method for setting up input queue
void AndroidInputPlatform::SetInputQueue(void* input_queue) {
    impl_->SetInputQueue(static_cast<AInputQueue*>(input_queue));
}

} // namespace Window  
} // namespace PyNovaGE

#endif // PYNOVAGE_PLATFORM_ANDROID