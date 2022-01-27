#pragma once
#include <GL/glew.h>
#include <string>

class Shader
{
public:
	Shader();
	~Shader();

	/**
	 * @brief シェーダープログラムをロードする
	 *
	 * @param vShaderFileName バーテックスシェーダーファイルのパス
	 * @param fShaderFileName フラグメントシェーダーファイルのパス
	 * @return true ロード成功
	 * @return false ロード失敗
	 */
	bool Load(const std::string& vShaderFileName, const std::string& fShaderFileName);

	/**
	 * @brief シェーダープログラム、オブジェクトの破棄
	 *
	 */
	void Unload();

	/**
	 * @brief シェーダープログラムを有効にする
	 *
	 */
	void SetActive();

	/**
	 * @brief シェーダープログラムに4x4行列を送る
	 *
	 * @param name 送り先のuniform変数名
	 * @param matrix 送る行列
	 */
	bool SetMatrixUniform(const std::string& name, const float* matrix);

	/**
	 * @brief シェーダープログラムに3次元ベクトルを送る
	 *
	 * @param name 送り先のuniform変数名
	 * @param vector 送る3次元ベクトル
	 * @return true
	 * @return false
	 */
	bool SetVector3Uniform(const std::string& name, const float* vector);

	/**
	 * @brief シェーダープログラムに1つの float 型の値を送る
	 *
	 * @param name 送り先のuniform変数名
	 * @param value 送る値
	 * @return true
	 * @return false
	 */
	bool SetFloatUniform(const std::string& name, const float value);

	/**
	 * @brief シェーダープログラムに1つの int 型の値を送る(テクスチャのセットにも使える)
	 *
	 * @param name uniform変数名
	 * @param value 送る値(テクスチャならば)
	 * @return true
	 * @return false
	 */
	bool SetIntUniform(const std::string& name, const int value);

	const GLuint GetShaderProgramID() const { return mShaderProgramID; }

private:
	/**
	 * @brief シェーダーファイルのコンパイルを行う
	 *
	 * @param fileName シェーダーファイル名
	 * @param shaderType シェーダーの種類
	 * @param outShader シェーダーのID(参照)
	 * @return true コンパイル成功
	 * @return false コンパイル失敗
	 */
	bool CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader);

	/**
	 * @brief シェーダーファイルのコンパイルが成功したか否かを判定
	 *
	 * @param shader 成功判定を調べるシェーダープログラムのID
	 * @return true 成功
	 * @return false 失敗
	 */
	bool IsCompiled(GLuint shader);

	/**
	 * @brief シェーダープログラムが正しくリンクできたかを調べる
	 *
	 * @param programID 調べるシェーダープログラム
	 * @return true 成功
	 * @return false 失敗
	 */
	bool IsValidProgram(const GLuint programID);

private:
	GLuint mVertShaderID;
	GLuint mFragShaderID;
	GLuint mShaderProgramID;
};