
float _determinant2(mat2 m) {    return m[0][0] * m[1][1] - m[0][1] * m[1][0];}out vec4 sk_FragColor;
uniform vec2 ah;
uniform vec2 bh;
uniform vec2 af;
uniform vec2 bf;
void main() {
    sk_FragColor.x = false ? _determinant2(mat2(ah, bh)) : ah.x * bh.y - ah.y * bh.x;
    sk_FragColor.y = false ? _determinant2(mat2(af, bf)) : af.x * bf.y - af.y * bf.x;
    sk_FragColor.z = 12.0;
    sk_FragColor.xyz = vec3(-8.0, -8.0, 12.0);
    sk_FragColor.yzw = vec3(9.0, -18.0, -9.0);
}
