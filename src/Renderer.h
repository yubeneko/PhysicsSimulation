#pragma once

#include <SDL.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>

class Renderer
{
public:
	enum ShaderType
	{
		Depth,
		ShadowMapping,
	};

	struct DirectionalLightProperty
	{
		const float mLightDistance = 20.0f;
		glm::vec3 mLightDirection{-0.707107f, -0.707107f, 0.0f};
		glm::vec3 mLightPos = -1.0f * mLightDistance * mLightDirection;
		glm::vec3 mAmbient{0.2f, 0.2f, 0.2f};
		glm::vec3 mDiffuse{1.0f, 1.0f, 1.0f};
		glm::vec3 mSpecular{1.0f, 1.0f, 1.0f};
	};

	Renderer(class Core& core);
	~Renderer();

	bool Initialize(float screenWidth, float screenHeight);
	void Shutdown();

	void Draw();
	void Draw3DScene(unsigned int frameBuffer, glm::mat4 view, glm::mat4 projection, float viewPortScale = 1.0f);
	void DrawShadowMap();

	float GetScreenWidth() const { return mScreenWidth; }
	float GetScreenHeight() const { return mScreenHeight; }

	std::weak_ptr<class Texture> GetTexture(const std::string& filePath);

	void AddMeshComponent(std::weak_ptr<class MeshComponent> meshComp);
	std::weak_ptr<class Mesh> GetMesh(const std::string& path);

	void SetViewMatrix(const glm::mat4& mat) { mView = mat; }
	void SetCameraPos(const glm::vec3& pos) { mCameraPos = pos; }

	bool CreateRenderTexture();
	std::weak_ptr<class Texture> GetRenderTexture() const { return mRenderTexture; }

	bool CreateShadowMapTexture();
	std::weak_ptr<class Texture> GetShadowMapTexture() const { return mShadowMapTexture; }

private:
	bool LoadShaders();
	void SetLightUniforms(class Shader& shader);

	/**
	 * @brief 平行光源の光の指す方向と位置をセットする。
	 *
	 * @param direction 光が差す方向の単位ベクトル
	 * @param up 光が差す方向ベクトルに対する上向き(y軸)の鉛直方向単位ベクトル
	 */
	void SetDirLightDirectionAndPosition(glm::vec3 direction, glm::vec3 up);

	class Core& mCore;

	float mScreenWidth;
	float mScreenHeight;

	SDL_Window* mWindow;
	SDL_GLContext mContext;

	// 読み込んだ画像データをTextureにしたものを格納
	std::unordered_map<std::string, std::shared_ptr<class Texture>> mTextures;

	// 読み込んだメッシュの連想配列
	std::unordered_map<std::string, std::shared_ptr<class Mesh>> mMeshes;

	// ビュー行列と投影行列
	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::vec3 mCameraPos;

	// シェーダーのIDとシェーダーの連想配列
	// ※ このIDはShaderのプログラムIDとは別物
	std::map<int, std::unique_ptr<class Shader>> mShaderDict;
	// シェーダーごとのメッシュのリスト
	std::map<int, std::vector<std::weak_ptr<class MeshComponent>>> mMeshCompDict;

	// RenderTexture用のフレームバッファオブジェクト
	unsigned int mFrameBufferID_ForRenderTexture;
	// RenderTexture
	std::shared_ptr<class Texture> mRenderTexture;

	// 平行光源のプロパティ
	DirectionalLightProperty mDirLightProp;

	// シャドウマッピングに必要なもの
	// 光源の方向を保持するアクター、ビュー行列、投影行列
	glm::mat4 mDirLightViewMat;
	glm::mat4 mDirLightProjMat;

	unsigned int mFrameBufferID_ForShadowMap;
	std::shared_ptr<class Texture> mShadowMapTexture;
};