uniform float uMode;
uniform vec2  uResolution;
uniform vec4  uControl;
uniform vec2  uMoilSize;
uniform vec2  uMoilCenter;
uniform float uCpRatio;
uniform float p0, p1, p2, p3, p4, p5;
uniform sampler2D uTexture;

out vec4 fragColor;
#define PI 3.14159265358979323846

float calculateRho(float alpha) {
    float a2 = alpha * alpha; float a3 = a2 * alpha; float a4 = a3 * alpha;
    float a5 = a4 * alpha; float a6 = a5 * alpha;
    return (p0*a6 + p1*a5 + p2*a4 + p3*a3 + p4*a2 + p5*alpha) * uCpRatio;
}

float mitchellNetravali(float x) {
    const float B = 1.0 / 3.0; const float C = 1.0 / 3.0;
    x = abs(x);
    if (x < 1.0) return ((12.0 - 9.0*B - 6.0*C)*x*x*x + (-18.0 + 12.0*B + 6.0*C)*x*x + (6.0 - 2.0*B)) / 6.0;
    else if (x < 2.0) return ((-B - 6.0*C)*x*x*x + (6.0*B + 30.0*C)*x*x + (-12.0*B - 48.0*C)*x + (8.0*B + 24.0*C)) / 6.0;
    return 0.0;
}

vec4 sampleBicubic(vec2 uv) {
    vec2 texel = 1.0 / uMoilSize;
    vec2 coord = uv * uMoilSize - 0.5;
    vec2 fxy   = fract(coord); coord = floor(coord);
    vec4 xcubic = vec4(mitchellNetravali(fxy.x + 1.0), mitchellNetravali(fxy.x), mitchellNetravali(1.0 - fxy.x), mitchellNetravali(2.0 - fxy.x));
    vec4 ycubic = vec4(mitchellNetravali(fxy.y + 1.0), mitchellNetravali(fxy.y), mitchellNetravali(1.0 - fxy.y), mitchellNetravali(2.0 - fxy.y));
    vec4 result = vec4(0.0);
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            vec2 sUV = clamp((coord + vec2(float(i) - 1.0, float(j) - 1.0) + 0.5) * texel, texel * 0.5, 1.0 - texel * 0.5);
            result += texture(uTexture, sUV) * xcubic[i] * ycubic[j];
        }
    }
    return result;
}

vec4 sampleWithAberration(vec2 uv) {
    const float strength = 0.0025; vec2 dir = uv - 0.5;
    vec2 uvR = clamp(uv + dir * strength, 0.0, 1.0);
    vec2 uvB = clamp(uv - dir * strength, 0.0, 1.0);
    return vec4(sampleBicubic(uvR).r, sampleBicubic(uv).g, sampleBicubic(uvB).b, 1.0);
}

float boundaryAlpha(vec2 texCoord) {
    vec2 edge = min(texCoord, 1.0 - texCoord);
    float fadePixels = 3.0 / min(uMoilSize.x, uMoilSize.y);
    return smoothstep(0.0, fadePixels, min(edge.x, edge.y));
}

float vignette(vec2 uvScreen) {
    vec2 d = uvScreen - 0.5;
    return clamp(pow(1.0 - dot(d, d) * 1.8, 3.0), 0.0, 1.0);
}

void main() {
    // [PERBAIKAN 1]: Balik sumbu Y. OpenGL Bottom-Left, Flutter Top-Left.
    vec2 fragCoord = vec2(gl_FragCoord.x, uResolution.y - gl_FragCoord.y);
    
    float sourceX = 0.0, sourceY = 0.0;
    const float PCT_W = 1.27, PCT_H = 1.27, FOCAL_ZOOM = 250.0;

    if (uMode <= 1.5) {
        float dcx = uResolution.x * 0.5; float dcy = uResolution.y * 0.5;
        float alphaOffset = uControl.x * (PI / 180.0);
        float betaOffset  = (uControl.y + (uMode < 0.5 ? 180.0 : 0.0)) * (PI / 180.0);
        float zoom = uControl.z;
        float tx, ty, tz;

        if (uMode < 0.5) {
            tx = (fragCoord.x - dcx) * (PCT_W * cos(betaOffset))  - (fragCoord.y - dcy) * (PCT_H * cos(alphaOffset) * sin(betaOffset)) + (FOCAL_ZOOM * zoom * sin(alphaOffset) * sin(betaOffset));
            ty = (fragCoord.x - dcx) * (PCT_W * sin(betaOffset))  + (fragCoord.y - dcy) * (PCT_H * cos(alphaOffset) * cos(betaOffset)) - (FOCAL_ZOOM * zoom * sin(alphaOffset) * cos(betaOffset));
            tz = (fragCoord.y - dcy) * (PCT_H * sin(alphaOffset)) + (FOCAL_ZOOM * zoom * cos(alphaOffset));
        } else {
            tx = -((fragCoord.x - dcx) * (PCT_W * cos(betaOffset))  + (fragCoord.y - dcy) * (PCT_H * sin(alphaOffset) * sin(betaOffset)) + (FOCAL_ZOOM * zoom * cos(alphaOffset) * sin(betaOffset)));
            ty = -((fragCoord.y - dcy) * (PCT_H * cos(alphaOffset)) - (FOCAL_ZOOM * zoom * sin(alphaOffset)));
            tz = -(fragCoord.x - dcx) * (PCT_W * sin(betaOffset))   + (fragCoord.y - dcy) * (PCT_H * sin(alphaOffset) * cos(betaOffset)) + (FOCAL_ZOOM * zoom * cos(alphaOffset) * cos(betaOffset));
        }
        float alpha = atan(sqrt(tx*tx + ty*ty), tz); float beta  = atan(ty, tx); float rho   = calculateRho(alpha);
        sourceX = uMoilCenter.x - rho * cos(beta); sourceY = uMoilCenter.y - rho * sin(beta);
    } 
    else if (uMode < 2.5) {
        float alphaMax = uControl.w * (PI / 180.0); float iC_alpha = uControl.x * (PI / 180.0); float iC_beta = -uControl.y * (PI / 180.0);
        float kx = sin(iC_alpha) * cos(iC_beta); float ky = sin(iC_alpha) * sin(iC_beta); float kz = cos(iC_alpha);
        float target_alpha = iC_alpha + ((fragCoord.y / uResolution.y) * alphaMax);
        float Vx = sin(target_alpha) * cos(iC_beta); float Vy = sin(target_alpha) * sin(iC_beta); float Vz = cos(target_alpha);
        float kxa_x = ky*Vz - kz*Vy; float kxa_y = kz*Vx - kx*Vz; float kxa_z = kx*Vy - ky*Vx; float k_a = kx*Vx + ky*Vy + kz*Vz;
        float ing_beta = (fragCoord.x / uResolution.x) * 2.0 * PI;
        float cB = cos(ing_beta); float sB = sin(ing_beta);
        float v_rot_x = cB*Vx + kxa_x*sB + kx*k_a*(1.0 - cB); float v_rot_y = cB*Vy + kxa_y*sB + ky*k_a*(1.0 - cB); float v_rot_z = cB*Vz + kxa_z*sB + kz*k_a*(1.0 - cB);
        float f_alpha = atan(sqrt(v_rot_x*v_rot_x + v_rot_y*v_rot_y), v_rot_z); float f_beta  = (PI / 2.0) - atan(v_rot_y, v_rot_x);
        float rho = calculateRho(f_alpha);
        sourceX = uMoilCenter.x - rho * cos(f_beta); sourceY = uMoilCenter.y - rho * sin(f_beta);
    }

    vec2 texCoord = vec2(sourceX, sourceY) / uMoilSize;
    
    // [PERBAIKAN 2]: Matikan Jebakan Merah (Kembalikan ke warna Hitam)
    if (texCoord.x < 0.0 || texCoord.x > 1.0 || texCoord.y < 0.0 || texCoord.y > 1.0) {
        // Angka 1.0 yang pertama diganti menjadi 0.0 agar warnanya hitam solid
        fragColor = vec4(0.0, 0.0, 0.0, 1.0); 
        return;
    }

    vec4 col = sampleWithAberration(texCoord);
    col = vec4(col.b, col.g, col.r, col.a); // Swap RGB to BGR
    col.rgb *= boundaryAlpha(texCoord);
    vec2 uvScreen = gl_FragCoord.xy / uResolution; // Gunakan gl_FragCoord untuk vignette
    col.rgb = mix(col.rgb, col.rgb * vignette(uvScreen), 0.35);
    col.rgb = (col.rgb - 0.5) * 1.05 + 0.5;
    float lum = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
    col.rgb = mix(vec3(lum), col.rgb, 1.08);

    fragColor = vec4(col.rgb, 1.0);
}