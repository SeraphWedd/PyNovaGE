#version 330 core

in vec2 v_uv;

// Camera matrices
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

// Sun parameters
uniform vec3 u_sun_direction;   // world-space direction to sun
uniform vec3 u_sun_color;
uniform float u_sun_intensity;
uniform float u_time;
uniform float u_sun_elevation;  // 0 = night, 1 = noon (camera-independent)

out vec4 FragColor;

// Convert world-space sun direction to view-space
vec3 sunViewDir() {
    // Treat as direction: use 3x3 portion of view
    mat3 viewRot = mat3(u_view_matrix);
    return normalize(viewRot * (-u_sun_direction)); // direction from camera to sun
}

// Hash for star field
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

void main() {
    vec3 sunV = sunViewDir();

    // Sun elevation factor from CPU (camera-independent)
    float sunElev = clamp(u_sun_elevation, 0.0, 1.0);

    // Time-of-day keyed sky gradient
    float h = clamp(v_uv.y, 0.0, 1.0);
    vec3 nightBottom = vec3(0.02, 0.03, 0.06);
    vec3 nightTop    = vec3(0.02, 0.04, 0.10);
    vec3 dawnBottom  = vec3(0.40, 0.20, 0.15);
    vec3 dawnTop     = vec3(0.85, 0.45, 0.25);
    vec3 dayBottom   = vec3(0.45, 0.65, 0.90);
    vec3 dayTop      = vec3(0.60, 0.80, 0.98);
    vec3 duskBottom  = vec3(0.40, 0.20, 0.15);
    vec3 duskTop     = vec3(0.85, 0.45, 0.25);

    // Match sky bands directly to sun position logic used on CPU
    // elev = clamp(-sun_dir.y, 0..1). When sun is below horizon (sun_dir.y > 0), it's night.
    float below = smoothstep(0.02, 0.10, u_sun_direction.y); // 0 above horizon, 1 well below
    float nightW = below;

    // Twilight when sun is just above horizon (low elevation) and not below horizon
    float twilightW = (1.0 - below) * (1.0 - smoothstep(0.15, 0.35, sunElev));

    // Day is the remainder
    float dayW = clamp(1.0 - nightW - twilightW, 0.0, 1.0);

    vec3 skyNight = mix(nightBottom, nightTop, pow(h, 1.6));
    vec3 skyTwilight = mix(dawnBottom, dawnTop, pow(h, 1.2));
    vec3 skyDay   = mix(dayBottom,   dayTop,   pow(h, 1.0));

    vec3 skyColor = vec3(0.0);
    skyColor += skyNight * nightW;
    skyColor += skyTwilight * twilightW;
    skyColor += skyDay * dayW;

    // Sun screen position (project unit vector)
    vec3 sunPointView = sunV;
    vec4 clip = u_projection_matrix * vec4(sunPointView, 1.0);
    vec2 sun_ndc = clip.xy / max(clip.w, 1e-4);
    vec2 uv_sun = sun_ndc * 0.5 + 0.5;

    float d = length(v_uv - uv_sun);

    // Warmer, stronger sun disc + halo
    float disc = smoothstep(0.045, 0.030, d);
    float halo = smoothstep(0.20, 0.08, d);
    vec3 sunColor = u_sun_color * (0.85 * disc + 0.35 * halo) * (u_sun_intensity * 1.25);

    // Night stars
    float nightFactor = nightW;
    float star = step(0.998, hash(v_uv * vec2(1024.0, 768.0)));
    float starGlow = smoothstep(0.002, 0.0005, abs(hash(v_uv * 2000.0) - 0.5));
    vec3 stars = vec3(star * starGlow) * 1.0;
    skyColor += stars * nightFactor;

    vec3 color = skyColor + sunColor;
    FragColor = vec4(color, 1.0);
}
