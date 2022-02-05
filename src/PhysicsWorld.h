#pragma once

#include "SimplePhysics/Spx.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	/**
	 * @brief 剛体を登録する
	 *
	 * @param rb RigiBody コンポーネント
	 * @return int 剛体のID
	 */
	int AddRigidbody(const class RigidBody& rb);

	/**
	 * @brief 1タイムステップ間における剛体シミュレーションを行う
	 *
	 */
	void Simulate();

	///////////////////////////////////////////////////////////////////////////////
	//
	// 剛体に関連するデータを取得する関数

	int GetNumRigidbodies() { return mNumRigidBodies; }
	const SimplePhysics::SpxState& GetState(int i) { return mStates[i]; }
	const SimplePhysics::SpxRigidBody& GetRigidbody(int i) { return mRigidbodies[i]; }
	const SimplePhysics::SpxCollidable& GetCollidable(int i) { return mCollidables[i]; }

	///////////////////////////////////////////////////////////////////////////////
	//
	// 剛体の属性を変更する関数

	void SetMotionType(int i, SimplePhysics::SpxMotionType type);

	///////////////////////////////////////////////////////////////////////////////
	//
	// 衝突情報を取得する関数

	int GetNumContacts() { return mNumPairs[mPairSwap]; }
	const SimplePhysics::SpxContact& GetContact(int i) { return *mPairs[mPairSwap][i].contact; }
	SimplePhysics::SpxUInt32 GetRigidbodyAInContact(int i) { return mPairs[mPairSwap][i].rigidBodyA; }
	SimplePhysics::SpxUInt32 GetRigidbodyBInContact(int i) { return mPairs[mPairSwap][i].rigidBodyB; }

private:
	///////////////////////////////////////////////////////////////////////////////
	//
	// シミュレーション定数

	// 最大剛体数
	static const inline int mMaxRigidBodies{500};
	// 最大ジョイント数
	static const inline int mMaxJoints{100};
	// 最大ペア数
	static const inline int mMaxPairs{5000};
	// シミュレーションのタイムステップ
	static const inline float mTimeStep{0.016f};
	// 拘束演算のイテレーション数
	static const inline int mIteration{10};
	// 位置補正のバイアス
	static const inline float mContactBias{0.1f};
	// 貫通許容誤差
	static const inline float mContactSlop{0.001f};
	// 重力
	static const inline glm::vec3 mGravity{0.0f, -9.8f, 0.0f};

	///////////////////////////////////////////////////////////////////////////////
	//
	// シミュレーションデータ

	// 剛体
	SimplePhysics::SpxState mStates[mMaxRigidBodies];
	SimplePhysics::SpxRigidBody mRigidbodies[mMaxRigidBodies];
	SimplePhysics::SpxCollidable mCollidables[mMaxRigidBodies];
	SimplePhysics::SpxUInt32 mNumRigidBodies = 0;

	// ジョイント

	SimplePhysics::SpxBallJoint mJoints[mMaxJoints];
	SimplePhysics::SpxUInt32 mNumJoints = 0;

	// ペア

	unsigned int mPairSwap;
	SimplePhysics::SpxUInt32 mNumPairs[2];
	SimplePhysics::SpxPair mPairs[2][mMaxPairs];

	// 経過フレーム
	static inline unsigned long mFrame = 0ul;

	///////////////////////////////////////////////////////////////////////////////

	// アロケータ
	class DefaultAllocator : public SimplePhysics::SpxAllocator
	{
	public:
		void* allocate(size_t bytes) override
		{
			return malloc(bytes);
		}

		void deallocate(void* p) override
		{
			free(p);
		}
	};

	DefaultAllocator mAllocator;
};