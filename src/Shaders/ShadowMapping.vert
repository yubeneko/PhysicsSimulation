#version 330

layout(location = 0) in vec3 inVertexPos_modelSpace;
layout(location = 1) in vec3 inNormalVector_modelSpace;

uniform mat4 uModelMat; // モデル行列
uniform mat4 uViewProjMat; // ビュー射影行列
uniform mat4 uDepthBiasViewProjMat; // 深度のバイアスとなる行列

// 光源の位置をカメラの位置としたときのクリップ空間での頂点座標
out vec4 fragPos_lightClipSpace;
// フラグメントのワールド空間での座標
out vec3 fragPos_worldSpace;
// フラグメントのワールド空間での法線ベクトル
out vec3 fragNormal_worldSpace;

void main() {
	// 頂点のワールド座標
	vec3 pos_worldSpace = vec3(uModelMat * vec4(inVertexPos_modelSpace, 1.0));
	fragPos_worldSpace = pos_worldSpace;
	// 頂点のワールド空間での法線ベクトル
	// モデル行列のスケールが非一様であった場合でも法線が崩れないように、法線行列を作って使っている。
	// https://learnopengl.com/Lighting/Basic-Lighting
	fragNormal_worldSpace = transpose(inverse(mat3(uModelMat))) * inNormalVector_modelSpace;

	// クリッピング空間での光源から見た時の頂点の座標
	// (ただし、UV座標と出力を揃えるためのバイアスを掛けている)
	fragPos_lightClipSpace = uDepthBiasViewProjMat * vec4(pos_worldSpace, 1.0);

	// クリップ空間でのカメラから見たときの頂点座標
	gl_Position = uViewProjMat * vec4(pos_worldSpace, 1.0);
}