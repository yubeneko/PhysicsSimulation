#pragma once

#include <string>

class Texture
{
public:
	Texture();
	~Texture();

	/**
	 * @brief テクスチャををOpenGLにロードする
	 *
	 * @param fileName 画像ファイルのパス(プログラムを実行したディレクトリからの相対パス)
	 * @return true
	 * @return false
	 */
	bool Load(const std::string& fileName);
	void Unload();

	/**
	 * @brief レンダーテクスチャを作る
	 *
	 * @param width 幅
	 * @param height 高さ
	 * @param format フォーマット
	 */
	void CreateForRendering(int width, int height, unsigned int format);

	/**
	 * @brief 深度描画用のテクスチャを作る
	 *
	 * @param width 幅
	 * @param height 高さ
	 */
	void CreateForDepthRendering(int width, int height);

	void SetActive();

	unsigned int GetTextureID() const { return mTextureID; };
	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }

private:
	unsigned int mTextureID;
	int mWidth;
	int mHeight;
};