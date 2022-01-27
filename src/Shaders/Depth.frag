#version 330


void main ()
{
	// フラグメントでのz(深度値)を出力するだけ
	// 実は、深度を出力するだけならばフラグメントシェーダーは不要。
	// GL_DEPTH_COMPONENT16 の出力値は0~255で、出力のチャンネルは赤の1つだけ。
	// そのため、赤色で見える。

	// この処理は裏で自動的に行われている
	// gl_FragDepth = gl_FragCoord.z;

	// https://stackoverflow.com/questions/46141786/opengl-es3-framebuffer-draw-depth-in-red-scale
}