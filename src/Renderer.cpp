#include "Renderer.h"
#include "Core.h"
#include "Mesh.h"
#include "Shader.h"
#include "VertexArray.h"
#include "MeshComponent.h"
#include "Texture.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

#include <glm/gtx/string_cast.hpp>

Renderer::Renderer(Core& core)
  : mCore(core)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float screenWidth, float screenHeight)
{
	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	mWindow = SDL_CreateWindow(
		mCore.GetWindowName().c_str(),
		100,
		100,
		static_cast<int>(mScreenWidth),
		static_cast<int>(mScreenHeight),
		SDL_WINDOW_OPENGL);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	mContext = SDL_GL_CreateContext(mWindow);
	SDL_GL_MakeCurrent(mWindow, mContext);
	// SDL_GL_SetSwapInterval(1);	// Enable vsync

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		SDL_Log("Failed to initialize GLEW.");
		return false;
	}

	// On some platforms, GLEW will emit a benign error code,
	// so clear it
	glGetError();

	if (!LoadShaders())
	{
		SDL_Log("Failed to load shader programs.");
		return false;
	}

	// 投影行列とビュー行列を初期化
	mProjection = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(mScreenWidth) / static_cast<float>(mScreenHeight),
		0.1f,
		600.0f);

	// デバッグ
	// mProjection = glm::ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);

	mView = glm::lookAt(
		mCameraPos,
		glm::vec3(0, 0, 0),	  // 向いている方向
		glm::vec3(0, 1, 0));  // 上方向

	// RenderTexture を作成
	CreateRenderTexture();

	// ~~~~~ シャドウマッピング用の設定 ~~~~~
	CreateShadowMapTexture();
	// 光の方向を回転させるためのクォータニオン(ピッチ方向に45度、ヨー方向に45度)。
	glm::quat q = glm::identity<glm::quat>();
	q = glm::angleAxis(glm::radians(45.0f), glm::vec3(1, 0, 0)) * q;
	q = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)) * q;
	// 光源の方向ベクトルとその上向きのベクトル
	glm::vec3 dir = q * glm::vec3(0, 0, 1);
	// 光源の方向と座標を設定し、光源のビュー行列を作る
	SetDirLightDirectionAndPosition(dir, glm::vec3(0.0f,1.0f,0.0f));

	// 光源の投影行列を作る(平行光源なので平行投影)
	mDirLightProjMat = glm::ortho<float>(-10.0, 10.0f, -10.0f, 10.0f, 0.0f, 40.0f);

	// カリングを有効化する
	glEnable(GL_CULL_FACE);

	return true;
}

void Renderer::Shutdown()
{
	// メッシュ、テクスチャ、シェーダーのアンロードを行う

	for (auto i : mMeshes)
	{
		i.second->Unload();
	}

	for (auto i : mTextures)
	{
		i.second->Unload();
	}

	for (auto&& shader : mShaderDict)
	{
		shader.second->Unload();
	}

	// レンダーテクスチャ関連の破棄
	if (mRenderTexture)
	{
		glDeleteFramebuffers(1, &mFrameBufferID_ForRenderTexture);
		mRenderTexture->Unload();
	}

	// シャドウマップのためのテクスチャも破棄
	if (mShadowMapTexture)
	{
		glDeleteFramebuffers(1, &mFrameBufferID_ForShadowMap);
		mShadowMapTexture->Unload();
	}

	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
}

void Renderer::Draw()
{
	// シャドウマップを作成
	DrawShadowMap();
	// レンダーテクスチャ
	// Draw3DScene(mFrameBufferID_ForRenderTexture, mDirLightViewMat, mDirLightProjMat, 0.25f);
	// メインカメラが映すシーンの描画
	Draw3DScene(0, mView, mProjection);

	// デバッグのために光源の位置からの光景を描画してみる
	//Draw3DScene(0, mDirLightViewMat, mDirLightProjMat);

	SDL_GL_SwapWindow(mWindow);
}

void Renderer::Draw3DScene(unsigned int frameBuffer, glm::mat4 view, glm::mat4 projection, float viewPortScale)
{
	// これから書き込むフレームバッファをセット
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	// ビューポートの設定
	glViewport(0, 0,
			   static_cast<int>(mScreenWidth * viewPortScale),
			   static_cast<int>(mScreenHeight * viewPortScale));

	glClearColor(0.62f, 0.84f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// 以降メッシュの描画プロセス
	for (auto mc : mMeshCompDict)
	{
		// これから処理するシェーダーを利用するMeshComponentのリスト
		std::vector<std::weak_ptr<MeshComponent>> meshComps = mc.second;
		if (meshComps.empty()) { continue; }

		Shader& shader = *mShaderDict[mc.first];

		shader.SetActive();

		// ビュー射影行列をシェーダーに送る
		shader.SetMatrixUniform("uViewProjMat", glm::value_ptr(projection * view));

		// サーフェースシェーダーならば光源の位置などのデータもシェーダーに送る。
		// 実装して気づいたが、シェーダーが複数存在するならば、シェーダーの派生クラスを
		// 作って描画するメソッドを多態的に振る舞わせる方が自然だ。
		if (mc.first == ShaderType::ShadowMapping)
		{
			// 視点の座標をシェーダーに送る
			shader.SetVector3Uniform("uCameraPos_worldSpace", glm::value_ptr(mCameraPos));
			// ライト関連更新
			SetLightUniforms(shader);

			// 光源のビュー射影行列
			glm::mat4 depthBiasViewProj = mDirLightProjMat * mDirLightViewMat;
			shader.SetMatrixUniform("uDepthBiasViewProjMat", glm::value_ptr(depthBiasViewProj));
			glActiveTexture(GL_TEXTURE0);
			mShadowMapTexture->SetActive();
			shader.SetIntUniform("shadowMap", 0);
		}

		for (auto iter = meshComps.begin(); iter != meshComps.end();)
		{
			// メッシュコンポーネントの描画処理を呼び出す。
			// すでに破棄されたやつが含まれていた場合、それを削除する。
			auto meshComp = iter->lock();

			if (meshComp)
			{
				meshComp->Draw(shader);
				++iter;
			}
			// メッシュコンポーネントのリンク切れ
			else
			{
				iter = meshComps.erase(iter);
			}
		}
	}

	glDisable(GL_DEPTH_TEST);
}

void Renderer::DrawShadowMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID_ForShadowMap);
	glViewport(0, 0, mShadowMapTexture->GetWidth(), mShadowMapTexture->GetHeight());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	// シャドウマッピングのアーティファクトの1つであるピーターパンニングを除去するため
	// 表面を描画しなくする
	glCullFace(GL_FRONT);

	// 深度描画用シェーダーを有効化
	Shader& shader = *mShaderDict[Depth];
	shader.SetActive();

	// ビュー射影行列をシェーダーに送る(ここでは光源の視点からのものとなる)
	shader.SetMatrixUniform("uViewProjMat", glm::value_ptr(mDirLightProjMat * mDirLightViewMat));

	for (auto mc : mMeshCompDict)
	{
		// メッシュコンポーネントのリスト
		std::vector<std::weak_ptr<MeshComponent>> meshComps = mc.second;
		if (meshComps.empty()) { continue; }

		for (auto iter = meshComps.begin(); iter != meshComps.end();)
		{
			// メッシュコンポーネントの描画処理を呼び出す。
			// すでに破棄されたやつが含まれていた場合、それを削除する。
			auto meshComp = iter->lock();

			if (meshComp)
			{
				// 深度だけを描画
				meshComp->Draw(shader, true);
				++iter;
			}
			// メッシュコンポーネントのリンク切れ
			else
			{
				iter = meshComps.erase(iter);
			}
		}
	}

	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
}

std::weak_ptr<class Texture> Renderer::GetTexture(const std::string& filePath)
{
	auto iter = mTextures.find(filePath);
	if (iter != mTextures.end())
	{
		return std::weak_ptr<Texture>(iter->second);
	}

	// 見つからなかったのでロードする
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	if (texture->Load(filePath))
	{
		mTextures.emplace(filePath, texture);
		return std::weak_ptr<Texture>(texture);
	}

	// ロードに失敗
	SDL_Log("Failed to load Image file: %s", filePath.c_str());
	texture.reset();

	return std::weak_ptr<Texture>();
}

void Renderer::AddMeshComponent(std::weak_ptr<MeshComponent> meshComp)
{
	// 既存のリストに含まれていないか確認する
	// 存在したら取り除く
	for (auto&& dic : mMeshCompDict)
	{
		auto& meshComps = dic.second;
		const auto iter = std::find_if(
			meshComps.begin(), meshComps.end(),
			[&meshComp](const std::weak_ptr<MeshComponent>& mc) {
				return meshComp.lock() == mc.lock();
			});

		if (iter != meshComps.end())
		{
			// std::iter_swap(iter, meshComps.end() - 1);
			// meshComps.pop_back();
			meshComps.erase(iter);
		}
	}

	Renderer::ShaderType type = static_cast<Renderer::ShaderType>(meshComp.lock()->GetMaterial().mShaderType);
	mMeshCompDict[type].emplace_back(meshComp);
}

std::weak_ptr<Mesh> Renderer::GetMesh(const std::string& path)
{
	auto iter = mMeshes.find(path);
	if (iter != mMeshes.end())
	{
		return std::weak_ptr<Mesh>(iter->second);
	}

	// 見つからなかったのでロードする
	std::shared_ptr<Mesh> m = std::make_shared<Mesh>();
	if (m->Load(path))
	{
		mMeshes.emplace(path, m);
		return std::weak_ptr<Mesh>(m);
	}

	// ロードに失敗
	SDL_Log("Failed to load Mesh file: %s", path.c_str());
	m.reset();

	return std::weak_ptr<Mesh>();
}

void Renderer::SetLightUniforms(Shader& shader)
{
	// 平行光源の光線方向ベクトル
	shader.SetVector3Uniform("uDirectionalLight.direction_worldSpace", glm::value_ptr(mDirLightProp.mLightDirection));
	// 光源強度の環境光成分
	shader.SetVector3Uniform("uDirectionalLight.ambientColor", glm::value_ptr(mDirLightProp.mAmbient));
	// 光源強度の拡散反射光成分(光の色)
	shader.SetVector3Uniform("uDirectionalLight.diffuseColor", glm::value_ptr(mDirLightProp.mDiffuse));
	// 光源強度の鏡面反射成分
	shader.SetVector3Uniform("uDirectionalLight.specularColor", glm::value_ptr(mDirLightProp.mSpecular));
}

bool Renderer::LoadShaders()
{
	// シャドウマップ作成用シェーダー
	mShaderDict[Depth] = std::make_unique<Shader>();
	Shader& depthShader = *mShaderDict[Depth];
	bool ok = depthShader.Load("Shaders/Depth.vert", "Shaders/Depth.frag");
	if (!ok) { return false; }
	// このシェーダーは全てのメッシュコンポーネントを対象に描画するので、mMeshCompDict には追加しない。

	// シャドウマップを利用した影を考慮したシェーダー
	mShaderDict[ShadowMapping] = std::make_unique<Shader>();
	Shader& shadowMapping = *mShaderDict[ShadowMapping];
	ok = shadowMapping.Load("Shaders/ShadowMapping.vert", "Shaders/ShadowMapping.frag");
	if (!ok) { return false; }
	mMeshCompDict.insert(std::make_pair(ShadowMapping, std::vector<std::weak_ptr<MeshComponent>>()));

	return true;
}

bool Renderer::CreateRenderTexture()
{
	// 生成するテクスチャのサイズ(画面サイズの1/4とする)
	int width = static_cast<int>(mScreenWidth) / 4;
	int height = static_cast<int>(mScreenHeight) / 4;

	// ~~~~~~~~~ フレームバッファの作成処理 ~~~~~~~~~
	glGenFramebuffers(1, &mFrameBufferID_ForRenderTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID_ForRenderTexture);

	// ~~~~~~~~~ RenderTexture 用 Texture 生成処理 ~~~~~~~~~
	mRenderTexture = std::make_shared<Texture>();
	mRenderTexture->CreateForRendering(width, height, GL_RGB);

	// ~~~~~~~~~ 作成したフレームバッファに深度バッファを追加する処理 ~~~~~~~~~
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	// ~~~~~~~~~ Render Texture の設定に関する処理 ~~~~~~~~~
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mRenderTexture->GetTextureID(), 0);

	// フレームバッファの描画先のリストを設定する
	GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
	// 描画するカラーバッファのリストを指定する
	glDrawBuffers(1, drawBuffers);

	// フレームバッファの設定がうまくできているか確認
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glDeleteFramebuffers(1, &mFrameBufferID_ForRenderTexture);
		mRenderTexture->Unload();
		mRenderTexture.reset();
		return false;
	}

	return true;
}

void Renderer::SetDirLightDirectionAndPosition(glm::vec3 direction, glm::vec3 up)
{
	mDirLightProp.mLightDirection = direction;
	mDirLightProp.mLightPos = -1.0f * mDirLightProp.mLightDistance * direction;
	mDirLightViewMat = glm::lookAt(
		mDirLightProp.mLightPos,					   // 光源の位置
		mDirLightProp.mLightPos + direction * 100.0f,  // 光源の方向
		up);										   // 光源方向に対する上方向のベクトル
}

bool Renderer::CreateShadowMapTexture()
{
	glGenFramebuffers(1, &mFrameBufferID_ForShadowMap);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID_ForShadowMap);

	mShadowMapTexture = std::make_shared<Texture>();
	// 1024 x 1204 のシャドウマップを作る。
	mShadowMapTexture->CreateForDepthRendering(1024, 1024);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowMapTexture->GetTextureID(), 0);

	// カラーバッファは描画しない
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glDeleteFramebuffers(1, &mFrameBufferID_ForShadowMap);
		mShadowMapTexture->Unload();
		mShadowMapTexture.reset();
		return false;
	}

	return true;
}