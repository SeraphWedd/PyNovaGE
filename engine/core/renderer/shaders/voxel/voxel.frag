#version 330 core

// Input from vertex shader
in vec3 v_world_position;    // World space position
in vec3 v_view_position;     // View space position
in vec3 v_normal;            // World space normal
in vec2 v_tex_coords;        // Texture coordinates
in float v_light_level;      // Interpolated light level (0-15)
in float v_ao_factor;        // Ambient occlusion factor
flat in int v_voxel_type;    // Voxel type (flat)
in float v_fog_factor;       // Fog interpolation factor
in vec3 v_sun_light;         // Sun light contribution
in vec3 v_ambient_light;     // Ambient light contribution

// Textures
uniform sampler2DArray u_texture_array;     // Voxel texture array
uniform sampler2D u_noise_texture;          // Noise texture for effects
uniform sampler2D u_normal_map;             // Normal mapping (optional)

// Material properties
uniform float u_material_roughness;   // Surface roughness
uniform float u_material_metallic;    // Metallic factor
uniform float u_material_emission;    // Emission strength
uniform vec3 u_material_emission_color; // Emission color

// Voxel-specific uniforms
// uniform int u_voxel_type;               // Replaced by per-vertex flat v_voxel_type
uniform bool u_use_texture_arrays;   // Enable texture array sampling
uniform float u_texture_blend_factor; // Texture blending strength
uniform bool u_enable_normal_mapping; // Enable normal mapping
uniform float u_normal_strength;      // Normal map intensity

// Lighting settings (individual uniforms for OpenGL 3.3)
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

// Camera data for calculations
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_view_projection_matrix;
uniform vec3 u_camera_position;
uniform float u_near_plane;
uniform float u_far_plane;
uniform float u_fov;
uniform vec2 u_viewport_size;

// Debug and rendering options
uniform bool u_show_wireframe;      // Show wireframe overlay
uniform bool u_show_normals;        // Visualize normals
uniform bool u_show_ao;             // Visualize ambient occlusion
uniform bool u_show_light_levels;   // Visualize light levels
uniform vec3 u_wireframe_color; // Wireframe color

// Output
out vec4 FragColor;

// Utility functions
vec3 gammaCorrect(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}

vec3 reinhard(vec3 color) {
    return color / (color + 1.0);
}

// Simple PBR-style lighting calculation
vec3 calculatePBRLighting(vec3 albedo, vec3 normal, vec3 view_dir, vec3 light_dir, vec3 light_color) {
    // Lambertian diffuse
    float NdotL = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = albedo * NdotL;
    
    // Simple Blinn-Phong specular
    vec3 half_dir = normalize(light_dir + view_dir);
    float NdotH = max(dot(normal, half_dir), 0.0);
    float roughness = u_material_roughness;
    float shininess = 2.0 / (roughness * roughness) - 2.0;
    vec3 specular = vec3(pow(NdotH, shininess)) * (1.0 - roughness);
    
    return (diffuse + specular * u_material_metallic) * light_color;
}

// Voxel light level to brightness conversion
float lightLevelToBrightness(float light_level) {
    // Convert Minecraft-style light levels (0-15) to brightness
    float normalized = light_level / 15.0;
    return normalized * normalized; // Quadratic falloff for more dramatic lighting
}

// Dithering for smooth gradients
float dither4x4(vec2 position, float brightness) {
    int x = int(mod(position.x, 4.0));
    int y = int(mod(position.y, 4.0));
    
    float dither_matrix[16] = float[16](
        0.0625, 0.5625, 0.1875, 0.6875,
        0.8125, 0.3125, 0.9375, 0.4375,
        0.25,   0.75,   0.125,  0.625,
        1.0,    0.5,    0.875,  0.375
    );
    
    float threshold = dither_matrix[y * 4 + x];
    return brightness > threshold ? 1.0 : 0.0;
}

vec3 voxelAlbedo(int t) {
    // Simple debug colors by voxel type
    if (t == 1) return vec3(0.55, 0.55, 0.6);   // STONE
    if (t == 2) return vec3(0.45, 0.30, 0.18);  // DIRT
    if (t == 3) return vec3(0.15, 0.55, 0.20);  // GRASS
    if (t == 4) return vec3(0.45, 0.3, 0.15);   // WOOD
    if (t == 5) return vec3(0.2, 0.5, 0.2);     // LEAVES
    return vec3(0.7); // default
}

void main() {
    // Lighting prepass
    vec3 light = v_sun_light + v_ambient_light;

    // Sample texture array if enabled; otherwise use debug voxel colors
    vec3 albedo;
    if (u_use_texture_arrays) {
        vec4 texel = texture(u_texture_array, vec3(v_tex_coords, float(v_voxel_type)));
        // Alpha test for cutout textures (e.g., leaves)
        if (texel.a < 0.5) discard;
        albedo = texel.rgb;
    } else {
        albedo = voxelAlbedo(v_voxel_type);
    }

    vec3 final_color = albedo * light;
    
    // Apply ambient occlusion
    final_color *= v_ao_factor;
    
    // Apply light level
    float brightness = lightLevelToBrightness(v_light_level);
    final_color *= brightness;
    
    // Apply fog
    if (u_enable_fog) {
        final_color = mix(u_fog_color, final_color, v_fog_factor);
    }
    
    // Gamma correction
    final_color = gammaCorrect(final_color, u_gamma);
    
    // Fallback tint if something is off
    if (length(final_color) < 0.01) {
        final_color = vec3(0.7, 0.7, 0.7);
    }
    
    FragColor = vec4(final_color, 1.0);
}

// Additional fragment shader utilities for future features:

/*
// Shadow mapping support
uniform sampler2D u_shadow_map;
in vec4 v_shadow_coord;

float calculateShadow(vec4 shadow_coord, float bias) {
    vec3 proj_coords = shadow_coord.xyz / shadow_coord.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    
    float closest_depth = texture(u_shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;
    
    return current_depth - bias > closest_depth ? 0.0 : 1.0;
}

// Percentage Closer Filtering (PCF)
float calculateShadowPCF(vec4 shadow_coord, vec2 texel_size, float bias) {
    vec3 proj_coords = shadow_coord.xyz / shadow_coord.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcf_depth = texture(u_shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += proj_coords.z - bias > pcf_depth ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

// Volumetric fog calculation
vec3 calculateVolumetricFog(vec3 world_pos, vec3 camera_pos, vec3 light_dir) {
    vec3 ray_dir = normalize(world_pos - camera_pos);
    float ray_length = length(world_pos - camera_pos);
    
    // Simple volumetric scattering approximation
    float scattering = max(dot(ray_dir, -light_dir), 0.0);
    scattering = pow(scattering, 8.0); // Sharp forward scattering
    
    return u_fog_color * scattering * u_fog_density;
}

// Screen-space ambient occlusion (SSAO) sampling
uniform sampler2D u_ssao_texture;

float sampleSSAO(vec2 screen_coords) {
    return texture(u_ssao_texture, screen_coords).r;
}

// Triplanar texture mapping for seamless voxel texturing
vec3 triplanarMapping(sampler2DArray tex_array, vec3 world_pos, vec3 normal, float layer) {
    vec3 blend = abs(normal);
    blend = normalize(max(blend, 0.00001));
    float b = (blend.x + blend.y + blend.z);
    blend /= vec3(b, b, b);
    
    vec3 x_axis = texture(tex_array, vec3(world_pos.yz, layer)).rgb;
    vec3 y_axis = texture(tex_array, vec3(world_pos.xz, layer)).rgb;
    vec3 z_axis = texture(tex_array, vec3(world_pos.xy, layer)).rgb;
    
    return x_axis * blend.x + y_axis * blend.y + z_axis * blend.z;
}
*/