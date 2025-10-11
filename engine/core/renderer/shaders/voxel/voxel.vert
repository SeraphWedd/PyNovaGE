#version 330 core

// Vertex attributes
layout(location = 0) in vec3 a_position;     // World position
layout(location = 1) in vec3 a_normal;       // Face normal
layout(location = 2) in vec2 a_tex_coords;   // Texture coordinates
layout(location = 3) in float a_voxel_type; // Encoded voxel type per-vertex
layout(location = 4) in float a_ao_factor;   // Ambient occlusion factor

// Camera matrices
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_view_projection_matrix;
uniform vec3 u_camera_position;
uniform float u_near_plane;
uniform float u_far_plane;
uniform float u_fov;
uniform vec2 u_viewport_size;

// Model matrix for chunk positioning
uniform mat4 u_model_matrix;

// Lighting uniforms
uniform vec3 u_sun_direction;      // Direction to sun
uniform vec3 u_sun_color;          // Sun light color
uniform float u_sun_intensity;     // Sun light intensity
uniform vec3 u_ambient_color;      // Ambient light color
uniform float u_ambient_intensity; // Ambient light intensity
uniform float u_gamma;             // Gamma correction value
uniform bool u_enable_fog;         // Enable distance fog
uniform vec3 u_fog_color;          // Fog color
uniform float u_fog_density;       // Fog density
uniform float u_fog_start;         // Fog start distance
uniform float u_fog_end;           // Fog end distance

// Voxel rendering settings
uniform float u_texture_scale;        // UV scale factor
uniform float u_time;                 // Time for animations
uniform bool u_enable_lighting;      // Enable lighting calculations
uniform bool u_enable_shadows;      // Enable shadow mapping
uniform bool u_wireframe_mode;      // Wireframe debugging

// Output to fragment shader
out vec3 v_world_position;    // World space position
out vec3 v_view_position;     // View space position
out vec3 v_normal;            // World space normal
out vec2 v_tex_coords;        // Texture coordinates
out float v_light_level;      // Interpolated light level
out float v_ao_factor;        // Ambient occlusion factor
flat out int v_voxel_type;    // Voxel type (flat, not interpolated)
out float v_fog_factor;       // Fog interpolation factor
out vec3 v_sun_light;         // Sun light contribution
out vec3 v_ambient_light;     // Ambient light contribution

void main() {
    // Transform vertex position using standard MVP pipeline
    vec4 world_pos = u_model_matrix * vec4(a_position, 1.0);
    vec4 view_pos = u_view_matrix * world_pos;
    
    // ALWAYS use consistent transformation - no wireframe conditional overrides
    gl_Position = u_projection_matrix * view_pos;
    
    // Pass world and view positions
    v_world_position = world_pos.xyz;
    v_view_position = view_pos.xyz;
    
    // Transform normal to world space
    mat3 normal_matrix = transpose(inverse(mat3(u_model_matrix)));
    v_normal = normalize(normal_matrix * a_normal);
    
    // Pass texture coordinates with scaling
    v_tex_coords = a_tex_coords * u_texture_scale;
    
    // Pass lighting attributes (use constant full brightness for now)
    v_light_level = 15.0; // full light level
    v_ao_factor = a_ao_factor;
    
    // Pass voxel type (as flat int)
    v_voxel_type = int(a_voxel_type + 0.5);
    
    // Calculate lighting contributions
    if (u_enable_lighting) {
        // Sun light calculation (Lambertian diffuse)
        float sun_dot = max(dot(v_normal, -u_sun_direction), 0.0);
        v_sun_light = u_sun_color * u_sun_intensity * sun_dot;
        
        // Ambient light
        v_ambient_light = u_ambient_color * u_ambient_intensity;
    } else {
        v_sun_light = vec3(1.0);
        v_ambient_light = vec3(0.0);
    }
    
    // Calculate fog factor
    if (u_enable_fog) {
        float distance = length(v_view_position);
        
        // Linear fog
        v_fog_factor = clamp((u_fog_end - distance) / (u_fog_end - u_fog_start), 0.0, 1.0);
        
        // Alternative: Exponential fog
        // v_fog_factor = exp(-u_fog_density * distance);
        
        // Alternative: Exponential squared fog
        // float fog_coord = u_fog_density * distance;
        // v_fog_factor = exp(-fog_coord * fog_coord);
    } else {
        v_fog_factor = 1.0;
    }
}

// Additional vertex shader utilities for future features:

/*
// Shadow mapping support (when implemented)
layout(std140, binding = 2) uniform ShadowData {
    mat4 u_shadow_matrix;      // Light view-projection matrix
    vec2 u_shadow_map_size;    // Shadow map dimensions
    float u_shadow_bias;       // Shadow bias to prevent acne
    bool u_enable_pcf;         // Percentage closer filtering
};

out vec4 v_shadow_coord;       // Shadow map coordinates

// In main():
if (u_enable_shadows) {
    v_shadow_coord = u_shadow_matrix * world_pos;
}

// Instanced rendering support (for multiple chunks)
layout(location = 5) in mat4 a_model_matrix;  // Per-instance model matrix
layout(location = 9) in vec3 a_chunk_offset;  // Per-instance chunk offset

// Wave animation for water/liquid voxels
vec3 calculateWaveOffset(vec3 position, float time) {
    float wave1 = sin(position.x * 0.5 + time * 2.0) * 0.1;
    float wave2 = cos(position.z * 0.3 + time * 1.5) * 0.05;
    return vec3(0.0, wave1 + wave2, 0.0);
}

// Level-of-detail support
uniform float u_lod_distance = 100.0;
uniform int u_max_lod_level = 4;

int calculateLOD(float distance) {
    return clamp(int(distance / u_lod_distance), 0, u_max_lod_level);
}
*/