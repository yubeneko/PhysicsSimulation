#include "PhysicsWorld.h"
#include "Actor.h"
#include "RigidBody.h"

PhysicsWorld::PhysicsWorld() = default;

PhysicsWorld::~PhysicsWorld() = default;

int PhysicsWorld::AddRigidbody(const class RigidBody& rb)
{
	int id = mNumRigidBodies;
	mNumRigidBodies++;

	auto owner = rb.GetOwner();
	// 各種データを初期化
	mStates[id].Reset();
	mStates[id].m_motionType = SimplePhysics::SpxMotionType::SpxMotionTypeActive;
	mStates[id].m_position = owner.lock()->GetPosition();
	mStates[id].m_orientation = owner.lock()->GetRotation();

	mRigidbodies[id].Reset();
	mCollidables[id].Reset();

	SimplePhysics::SpxShape shape;
	shape.Reset();

	// キューブのメッシュデータ

	// 頂点数
	const int box_numVertices = 8;
	// clang-format off
	// 頂点座標()
	const float box_vertices[] = {
		-0.500000, -0.500000, 0.500000,
		0.500000, -0.500000, 0.500000,
		-0.500000, 0.500000, 0.500000,
		0.500000, 0.500000, 0.500000,
		-0.500000, 0.500000, -0.500000,
		0.500000, 0.500000, -0.500000,
		-0.500000, -0.500000, -0.500000,
		0.500000, -0.500000, -0.500000};

	// 頂点インデックスの数
	const int box_numIndices = 36;
	// 頂点インデックス(反時計回り方向が表になるようにする!)
	// 頂点座標(OpenGLでは、Y up の -Z forward の右手系なので、そこに注意!
	// それを考えて図に書いてみると、確かにインデックスは反時計回りに登録されていることがわかる
	// Blender から出力する時も、Y-up Z-forward にしろ、Y-up -Z-forward にしてもちゃんと
	// インデックスは反時計回りになっている。
	const unsigned short box_indices[] = {
		0, 1, 2,
		2, 1, 3,
		2, 3, 4,
		4, 3, 5,
		4, 5, 6,
		6, 5, 7,
		6, 7, 0,
		0, 7, 1,
		1, 7, 3,
		3, 7, 5,
		6, 0, 4,
		4, 0, 2};
	// clang-format on

	// メッシュを作る
	// 座標、スケール、回転は Actor のデータをもとにする
	SimplePhysics::SpxCreateConvexMesh(
		&shape.m_geometry,
		box_vertices, box_numVertices,
		box_indices, box_numIndices,
		owner.lock()->GetScale());

	// 形状を登録
	mCollidables[id].AddShape(shape);
	// 剛体の登録の完了
	mCollidables[id].Finish();

	return id;
}

void PhysicsWorld::Simulate()
{
	// バッファをスワップ
	mPairSwap = 1 - mPairSwap;

	// 外力の適用
	for (SimplePhysics::SpxUInt32 i = 0; i < mNumRigidBodies; i++)
	{
		glm::vec3 externalForce = mGravity * mRigidbodies[i].m_mass;
		glm::vec3 externalTorque(0.0f);
		SimplePhysics::SpxApplyExternalForce(mStates[i], mRigidbodies[i], externalForce, externalTorque, mTimeStep);
	}

	// ブロードフェーズ
	SimplePhysics::SpxBroadPhase(
		mStates, mCollidables, mNumRigidBodies,
		mPairs[1 - mPairSwap], mNumPairs[1 - mPairSwap],
		mPairs[mPairSwap], mNumPairs[mPairSwap],
		mMaxPairs, &mAllocator, nullptr, nullptr);

	// 衝突判定
	SimplePhysics::SpxDetectCollision(
		mStates, mCollidables, mNumRigidBodies,
		mPairs[mPairSwap], mNumPairs[mPairSwap]);

	// 拘束演算
	SimplePhysics::SpxSolveConstraints(
		mStates, mRigidbodies, mNumRigidBodies,
		mPairs[mPairSwap], mNumPairs[mPairSwap],
		mJoints, mNumJoints,
		mIteration, mContactBias, mContactSlop, mTimeStep, &mAllocator);

	// 位置更新
	SimplePhysics::SpxIntegrate(mStates, mNumRigidBodies, mTimeStep);

	// フレーム更新
	mFrame++;
}

void PhysicsWorld::SetMotionType(int i, SimplePhysics::SpxMotionType type)
{
	mStates[i].m_motionType = type;
}

void PhysicsWorld::ApplyImpulse(int i, glm::vec3 velocity)
{
	mStates[i].m_linearVelocity = velocity;
}