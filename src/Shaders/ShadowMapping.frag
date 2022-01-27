#version 330

struct DirectionalLight {
	vec3 direction_worldSpace;
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
};

struct MaterialProperty {
	vec3 color; // マテリアルの色
	vec3 specularColor; // マテリアルの鏡面反射色
	float smoothness; // 鏡面反射の余弦の指数部分
};

// 光源の位置を視点としたときのクリップ空間での頂点座標
in vec4 fragPos_lightClipSpace;
// フラグメントのワールド空間での座標
in vec3 fragPos_worldSpace;
// フラグメントのワールド空間での法線ベクトル
in vec3 fragNormal_worldSpace;

layout(location = 0) out vec4 outColor;

// シャドウマップ(テクスチャ)
uniform sampler2D shadowMap;
// 平行光源
uniform DirectionalLight uDirectionalLight;
// オブジェクトのマテリアル
uniform MaterialProperty uMaterial;
// カメラ(視点)のワールド座標
uniform vec3 uCameraPos_worldSpace;

float ShadowCalculation(vec4 fragPos_world) {
	// クリップ空間の3次元座標(正規化デバイス座標: 全ての座標が[-1,1]の範囲になっている)を得る
	// (平行投影だとw成分は1なので除算の意味が、透視投影(スポットライトで使う)の時はwが1じゃないことがあるので、こうしている)
	vec3 projCoords = fragPos_lightClipSpace.xyz / fragPos_lightClipSpace.w;

	// シャドウマップは[0,1]の範囲なので、正規化デバイス座標([-1,1])を[0,1]に変換する。
	// 0.5をかけて0.5を足せばよい。
	projCoords = projCoords * 0.5 + 0.5;

	// 光源を視点としてみたときのクリップ空間におけるフラグメントの座標が1より大きい場合は影をなくす
	if(projCoords.z > 1.0) {
		return 0.0;
	}

	// 現在のフラグメントから光源までの距離([0,1]の範囲)
	float currentDepth = projCoords.z;

	// シャドウマップからフラグメントの位置における深度値を得る([0,1]の範囲)
	float closestDepth = texture(shadowMap, projCoords.xy).r;

	// シャドウアクネ対策のシャドウバイアス
	float bias = 0.0003;
	// 現在のフラグメントの位置がシャドウマップから読み取った深度値よりも大きい場合、
	// そのフラグメントは影になっている!
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

void main() {
	// フラグメントの法線ベクトル
	vec3 N = normalize(fragNormal_worldSpace);
	// フラグメントから光源へ向かうベクトル
	vec3 L = -1.0 * normalize(uDirectionalLight.direction_worldSpace);

	// 最終出力色
	vec3 color = uDirectionalLight.ambientColor * uMaterial.color;

	float NdotL = dot(N, L);
	if(NdotL > 0) {
		// ~~~~~~ 拡散反射と鏡面反射の計算 ~~~~~~
		vec3 diffuse = uDirectionalLight.diffuseColor * uMaterial.color * NdotL;

		vec3 R = normalize(reflect(-L, N));
		vec3 V = normalize(uCameraPos_worldSpace - fragPos_worldSpace);

		float RdotV = pow(max(0.0, dot(R, V)), uMaterial.smoothness);
		vec3 specular = uDirectionalLight.specularColor * uMaterial.specularColor * RdotV;

		// ~~~~~~ 影を考慮した光の強度の計算 ~~~~~~
		float visibility = 1.0 - ShadowCalculation(fragPos_lightClipSpace);

		// ~~~~~~ 最終的なフラグメントのRGBカラー ~~~~~~
		color += visibility * (diffuse + specular);
	}

	outColor = vec4(color, 1.0);
}