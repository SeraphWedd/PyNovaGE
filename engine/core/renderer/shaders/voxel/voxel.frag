#version 450 core

// Input from vertex shader
in vec3 v_world_position;    // World space position
in vec3 v_view_position;     // View space position
in vec3 v_normal;            // World space normal
in vec2 v_tex_coords;        // Texture coordinates
in float v_light_level;      // Interpolated light level (0-15)
in float v_ao_factor;        // Ambient occlusion factor
in float v_fog_factor;       // Fog interpolation factor
in vec3 v_sun_light;         // Sun light contribution
in vec3 v_ambient_light;     // Ambient light contribution

// Textures
uniform sampler2DArray u_texture_array;     // Voxel texture array
uniform sampler2D u_noise_texture;          // Noise texture for effects
uniform sampler2D u_normal_map;             // Normal mapping (optional)

// Material properties
uniform float u_material_roughness = 0.8;   // Surface roughness
uniform float u_material_metallic = 0.0;    // Metallic factor
uniform float u_material_emission = 0.0;    // Emission strength
uniform vec3 u_material_emission_color = vec3(1.0); // Emission color

// Voxel-specific uniforms
uniform int u_voxel_type = 0;               // Current voxel type for texture selection
uniform bool u_use_texture_arrays = true;   // Enable texture array sampling
uniform float u_texture_blend_factor = 1.0; // Texture blending strength
uniform bool u_enable_normal_mapping = false; // Enable normal mapping
uniform float u_normal_strength = 1.0;      // Normal map intensity

// Lighting settings (matching vertex shader)
layout(std140, binding = 1) uniform LightingData {
    vec3 u_sun_direction;      // Direction to sun
    vec3 u_sun_color;          // Sun light color
    float u_sun_intensity;     // Sun light intensity
    vec3 u_ambient_color;      // Ambient light color
    float u_ambient_intensity; // Ambient light intensity
    float u_gamma;             // Gamma correction value
    bool u_enable_fog;         // Enable distance fog
    vec3 u_fog_color;          // Fog color
    float u_fog_density;       // Fog density
    float u_fog_start;         // Fog start distance
    float u_fog_end;           // Fog end distance
};

// Camera data for calculations
layout(std140, binding = 0) uniform CameraMatrices {
    mat4 u_view_matrix;
    mat4 u_projection_matrix;
    mat4 u_view_projection_matrix;
    vec3 u_camera_position;
    float u_near_plane;
    float u_far_plane;
    float u_fov;
    vec2 u_viewport_size;
};

// Debug and rendering options
uniform bool u_show_wireframe = false;      // Show wireframe overlay
uniform bool u_show_normals = false;        // Visualize normals
uniform bool u_show_ao = false;             // Visualize ambient occlusion
uniform bool u_show_light_levels = false;   // Visualize light levels
uniform vec3 u_wireframe_color = vec3(1.0, 1.0, 0.0); // Wireframe color

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

void main() {
    vec3 normal = normalize(v_normal);
    vec3 view_dir = normalize(u_camera_position - v_world_position);
    
    // Sample base color from texture array
    vec3 base_color;
    if (u_use_texture_arrays) {
        base_color = texture(u_texture_array, vec3(v_tex_coords, float(u_voxel_type))).rgb;
    } else {
        // Fallback solid colors based on voxel type
        if (u_voxel_type == 1) { // Stone
            base_color = vec3(0.5, 0.5, 0.5);
        } else if (u_voxel_type == 2) { // Dirt  
            base_color = vec3(0.6, 0.4, 0.2);
        } else if (u_voxel_type == 3) { // Grass
            base_color = vec3(0.3, 0.8, 0.3);
        } else { // Default/unknown
            base_color = vec3(1.0, 0.0, 1.0); // Magenta for debugging
        }
    }
    
    // Apply texture blending
    base_color = mix(base_color, vec3(1.0), 1.0 - u_texture_blend_factor);
    
    // Normal mapping (if enabled)
    if (u_enable_normal_mapping) {
        vec3 normal_map = texture(u_normal_map, v_tex_coords).rgb * 2.0 - 1.0;
        // Simple normal perturbation (proper tangent space would be better)
        normal = normalize(normal + normal_map * u_normal_strength);
    }
    
    // Calculate lighting
    vec3 final_color = vec3(0.0);
    
    // Ambient light
    final_color += v_ambient_light * base_color;
    
    // Sun light (directional)
    final_color += calculatePBRLighting(base_color, normal, view_dir, -u_sun_direction, v_sun_light);
    
    // Voxel light level contribution
    float voxel_brightness = lightLevelToBrightness(v_light_level);
    final_color += base_color * voxel_brightness * 0.5; // Scale down to prevent overbrightening
    
    // Apply ambient occlusion
    final_color *= v_ao_factor;
    
    // Add emission
    if (u_material_emission > 0.0) {
        final_color += u_material_emission_color * u_material_emission;
    }
    
    // Apply fog
    if (u_enable_fog) {
        final_color = mix(u_fog_color, final_color, v_fog_factor);
    }
    
    // Debug visualizations
    if (u_show_normals) {
        final_color = normal * 0.5 + 0.5; // Visualize normals as colors
    } else if (u_show_ao) {
        final_color = vec3(v_ao_factor); // Visualize AO
    } else if (u_show_light_levels) {
        final_color = vec3(voxel_brightness); // Visualize light levels
    }
    
    // Tone mapping
    final_color = reinhard(final_color);
    
    // Gamma correction
    final_color = gammaCorrect(final_color, u_gamma);
    
    FragColor = vec4(final_color, 1.0);
    
    // Wireframe overlay
    if (u_show_wireframe) {
        // Simple wireframe effect using screen-space derivatives
        vec2 grid = abs(fract(v_tex_coords * 16.0) - 0.5);
        float line = smoothstep(0.0, 0.05, min(grid.x, grid.y));
        FragColor.rgb = mix(u_wireframe_color, FragColor.rgb, line);
    }
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