
out vec4 sk_FragColor;
layout (binding = 0, set = 0) uniform sampler s;
layout (binding = 1, set = 0) uniform texture2D t;
layout (binding = 2, set = 0) uniform sampler2D cs;
layout (location = 1) in vec2 c;
vec4 combined_sampler_only_helper_h4Z(sampler2D cs_param) {
    return texture(cs_param, c);
}
vec4 bottom_helper_h4Tss(texture2D t_param, sampler s_param) {
    return texture(makeSampler2D(t_param, s_param), c);
}
vec4 helpers_helper_h4Tss(texture2D t_param, sampler s_param) {
    vec4 color = texture(makeSampler2D(t_param, s_param), c);
    return color.zyxw * bottom_helper_h4Tss(t_param, s_param);
}
vec4 helper_h4Tss(texture2D t_param, sampler s_param) {
    return helpers_helper_h4Tss(t_param, s_param);
}
void main() {
    sk_FragColor = helper_h4Tss(t, s) + combined_sampler_only_helper_h4Z(cs);
}
