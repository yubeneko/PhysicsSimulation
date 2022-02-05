#include "SpxConstraintSolver.h"
#include "../elements/SpxSloverBody.h"
#include "../collision/SpxVectorFunction.h"
#include "../glmExtension.h"

#include <glm/gtx/transform.hpp>

namespace SimplePhysics
{

void SpxSolveConstraints(
	SpxState* states,
	const SpxRigidBody* bodies,
	SpxUInt32 numRigidBodies,
	const SpxPair* pairs,
	SpxUInt32 numPairs,
	SpxBallJoint* joints,
	SpxUInt32 numJoints,
	SpxUInt32 iteration,
	float bias,
	float slop,
	float timeStep,
	SpxAllocator* allocator)
{
	// ソルバー用プロキシを作成
	SpxSolverBody* solverBodies = (SpxSolverBody*)allocator->allocate(sizeof(SpxSolverBody) * numRigidBodies);

	// ソルバーボディにパラメータをセットしていく
	for (SpxUInt32 i = 0; i < numRigidBodies; i++)
	{
		SpxState& state = states[i];
		const SpxRigidBody& body = bodies[i];
		SpxSolverBody& solverBody = solverBodies[i];

		solverBody.orientation = state.m_orientation;
		solverBody.deltaLinearVelocity = glm::vec3(0.0f);
		solverBody.deltaAngularVelocity = glm::vec3(0.0f);

		// ワールドに固定されたオブジェクトは質量無限大として扱われる。
		if (state.m_motionType == SpxMotionTypeStatic)
		{
			// 質量と慣性テンソルの逆数は1/∞ = 0 となる。
			solverBody.massInv = 0.0f;
			solverBody.inertiaInv = glm::mat3(0.0f);
		}
		else {
			solverBody.massInv = 1.0f / body.m_mass;
			glm::mat3 m = glm::toMat3(solverBody.orientation);
			// 慣性テンソルの逆行列(回転させる)
			solverBody.inertiaInv = m * inverse(body.m_inertia) * transpose(m);
		}
	}

	// 拘束のセットアップ(ボールジョイント)。
	for (SpxUInt32 i = 0; i < numJoints; i++)
	{
		SpxBallJoint& joint = joints[i];

		SpxState& stateA = states[joint.rigidBodyA];
		//const SpxRigidBody& bodyA = bodies[joint.rigidBodyA];
		SpxSolverBody& solverBodyA = solverBodies[joint.rigidBodyA];

		SpxState& stateB = states[joint.rigidBodyB];
		//const SpxRigidBody& bodyB = bodies[joint.rigidBodyB];
		SpxSolverBody& solverBodyB = solverBodies[joint.rigidBodyB];

		glm::vec3 rA = solverBodyA.orientation * joint.anchorA;
		glm::vec3 rB = solverBodyB.orientation * joint.anchorB;

		glm::vec3 positionA = stateA.m_position + rA;
		glm::vec3 positionB = stateB.m_position + rB;
		// 拘束軸となるベクトルを計算(P251)
		glm::vec3 direction = positionA - positionB;
		float distanceSqr = glm::length2(direction);

		if (distanceSqr < SPX_EPSILON * SPX_EPSILON)
		{
			joint.constraint.jacDiagInv = 0.0f;
			joint.constraint.rhs = 0.0f;
			joint.constraint.lowerLimit = -FLT_MAX;
			joint.constraint.upperLimit = FLT_MAX;
			joint.constraint.axis = glm::vec3(1.0f, 0.0f, 0.0f);
			continue;
		}

		float distance = glm::sqrt(distanceSqr);
		direction /= distance;

		// 衝突点における相対速度を計算する
		glm::vec3 velocityA = stateA.m_linearVelocity + cross(stateA.m_angularVelocity, rA);
		glm::vec3 velocityB = stateB.m_linearVelocity + cross(stateB.m_angularVelocity, rB);
		glm::vec3 relativeVelocity = velocityA - velocityB;

		// 拘束力の式の分母の部分(法線ベクトルの部分は除く)
		glm::mat3 K = glm::mat3(glm::scale(glm::mat4(1.0f), glm::vec3(solverBodyA.massInv + solverBodyB.massInv))) -
					  GLMExtension::CrossMatrix(rA) * solverBodyA.inertiaInv * GLMExtension::CrossMatrix(rA) -
					  GLMExtension::CrossMatrix(rB) * solverBodyB.inertiaInv * GLMExtension::CrossMatrix(rB);

		// 拘束力の分母部分
		float denom = glm::dot(K * direction, direction);
		joint.constraint.jacDiagInv = 1.0f / denom;
		// 拘束力の分子部分
		joint.constraint.rhs = -glm::dot(relativeVelocity, direction);  // velocity error
		joint.constraint.rhs -= joint.bias * distance / timeStep;  // position error
		// 拘束力f
		joint.constraint.rhs *= joint.constraint.jacDiagInv;

		joint.constraint.lowerLimit = -FLT_MAX;
		joint.constraint.upperLimit = FLT_MAX;
		joint.constraint.axis = direction;

		joint.constraint.accumImpulse = 0.0f;
	}

	// 拘束のセットアップ(衝突)。
	// 衝突の情報から拘束計算に必要なパラメータを抽出して拘束の情報に変換する。
	for (SpxUInt32 i = 0; i < numPairs; i++)
	{
		const SpxPair& pair = pairs[i];

		SpxState& stateA = states[pair.rigidBodyA];
		const SpxRigidBody& bodyA = bodies[pair.rigidBodyA];
		SpxSolverBody& solverBodyA = solverBodies[pair.rigidBodyA];

		SpxState& stateB = states[pair.rigidBodyB];
		const SpxRigidBody& bodyB = bodies[pair.rigidBodyB];
		SpxSolverBody& solverBodyB = solverBodies[pair.rigidBodyB];

		assert(pair.contact);

		// 摩擦係数は2つのオブジェクトの摩擦係数の合成値とする
		pair.contact->m_friction = glm::sqrt(bodyA.m_friction * bodyB.m_friction);

		// 衝突のペアでイテレーション
		for (SpxUInt32 j = 0; j < pair.contact->m_numContacts; j++)
		{
			SpxContactPoint& cp = pair.contact->m_contactPoints[j];

			// 接続点を剛体の姿勢に合わせて回転。
			// すると、オブジェクトの重心から衝突点に向かうベクトルに変化する。
			glm::vec3 rA = rotate(solverBodyA.orientation, cp.pointA);
			glm::vec3 rB = rotate(solverBodyB.orientation, cp.pointB);

			// 拘束力の式の分母の部分(法線ベクトルの部分は除く)
			glm::mat3 K = glm::mat3(glm::scale(glm::mat4(1.0f), glm::vec3(solverBodyA.massInv + solverBodyB.massInv))) -
						  GLMExtension::CrossMatrix(rA) * solverBodyA.inertiaInv * GLMExtension::CrossMatrix(rA) -
						  GLMExtension::CrossMatrix(rB) * solverBodyB.inertiaInv * GLMExtension::CrossMatrix(rB);

			glm::vec3 velocityA = stateA.m_linearVelocity + cross(stateA.m_angularVelocity, rA);
			glm::vec3 velocityB = stateB.m_linearVelocity + cross(stateB.m_angularVelocity, rB);
			// 衝突点における相対速度を求める
			glm::vec3 relativeVelocity = velocityA - velocityB;

			glm::vec3 tangent1, tangent2;

			// 衝突点の法線ベクトル(ワールド座標系)をベースに2つの基底ベクトルを作る
			SpxCalcTangentVector(cp.normal, tangent1, tangent2);

			// 反発係数。
			// 新規に発生した衝突でない場合、反発係数は0とする。
			float restitution = (pair.type == SpxPairTypeNew) ? 0.5f * (bodyA.m_restitution + bodyB.m_restitution) : 0.0f;

			// Normal方向の拘束力を計算
			{
				glm::vec3 axis = cp.normal;
				float denom = glm::dot(K * axis, axis);
				cp.constraints[0].jacDiagInv = 1.0f / denom;
				cp.constraints[0].rhs = -(1.0f + restitution) * glm::dot(relativeVelocity, axis);	  // velocity error(反発係数込み)
				cp.constraints[0].rhs -= (bias * glm::min(0.0f, cp.distance + slop)) / timeStep;  // position error(許容距離込み)
				cp.constraints[0].rhs *= cp.constraints[0].jacDiagInv;
				cp.constraints[0].lowerLimit = 0.0f;
				cp.constraints[0].upperLimit = FLT_MAX;
				cp.constraints[0].axis = axis;
			}

			// Tangent1方向(摩擦その1)の拘束力を計算
			{
				glm::vec3 axis = tangent1;
				float denom = glm::dot(K * axis, axis);
				cp.constraints[1].jacDiagInv = 1.0f / denom;
				cp.constraints[1].rhs = -glm::dot(relativeVelocity, axis);
				cp.constraints[1].rhs *= cp.constraints[1].jacDiagInv;
				// 拘束力の下限と上限は反発方向の拘束力が分からないと決められないので、
				// とりあえず0で初期化しておく。
				cp.constraints[1].lowerLimit = 0.0f;
				cp.constraints[1].upperLimit = 0.0f;
				cp.constraints[1].axis = axis;
			}

			// Tangent2方向(摩擦その2)の拘束力を計算
			{
				glm::vec3 axis = tangent2;
				float denom = glm::dot(K * axis, axis);
				cp.constraints[2].jacDiagInv = 1.0f / denom;
				cp.constraints[2].rhs = -glm::dot(relativeVelocity, axis);
				cp.constraints[2].rhs *= cp.constraints[2].jacDiagInv;
				cp.constraints[2].lowerLimit = 0.0f;
				cp.constraints[2].upperLimit = 0.0f;
				cp.constraints[2].axis = axis;
			}
		}
	}

	// Warm starting
	// 衝突に関する各拘束力の初期値を0ではなく、過去の拘束力として与える。
	for (SpxUInt32 i = 0; i < numPairs; i++)
	{
		const SpxPair& pair = pairs[i];

		SpxSolverBody& solverBodyA = solverBodies[pair.rigidBodyA];
		SpxSolverBody& solverBodyB = solverBodies[pair.rigidBodyB];

		for (SpxUInt32 j = 0; j < pair.contact->m_numContacts; j++)
		{
			SpxContactPoint& cp = pair.contact->m_contactPoints[j];
			glm::vec3 rA = solverBodyA.orientation * cp.pointA;
			glm::vec3 rB = solverBodyB.orientation * cp.pointB;

			// 1つの衝突につき、3つの拘束がある
			for (SpxUInt32 k = 0; k < 3; k++)
			{
				float deltaImpulse = cp.constraints[k].accumImpulse;
				// それぞれのソルバーボディの並進速度の変化分と回転速度の変化分を計算する
				solverBodyA.deltaLinearVelocity += deltaImpulse * solverBodyA.massInv * cp.constraints[k].axis;
				solverBodyA.deltaAngularVelocity += deltaImpulse * solverBodyA.inertiaInv * cross(rA, cp.constraints[k].axis);
				solverBodyB.deltaLinearVelocity -= deltaImpulse * solverBodyB.massInv * cp.constraints[k].axis;
				solverBodyB.deltaAngularVelocity -= deltaImpulse * solverBodyB.inertiaInv * cross(rB, cp.constraints[k].axis);
			}
		}
	}

	// 拘束の演算
	// 指定したイテレーション回数だけ演算を繰り返す
	for (SpxUInt32 itr = 0; itr < iteration; itr++)
	{
		// ボールジョイントの拘束の計算
		for (SpxUInt32 i = 0; i < numJoints; i++)
		{
			SpxBallJoint& joint = joints[i];

			SpxSolverBody& solverBodyA = solverBodies[joint.rigidBodyA];
			SpxSolverBody& solverBodyB = solverBodies[joint.rigidBodyB];

			glm::vec3 rA = solverBodyA.orientation * joint.anchorA;
			glm::vec3 rB = solverBodyB.orientation * joint.anchorB;

			SpxConstraint& constraint = joint.constraint;
			float deltaImpulse = constraint.rhs;
			glm::vec3 deltaVelocityA = solverBodyA.deltaLinearVelocity + cross(solverBodyA.deltaAngularVelocity, rA);
			glm::vec3 deltaVelocityB = solverBodyB.deltaLinearVelocity + cross(solverBodyB.deltaAngularVelocity, rB);
			deltaImpulse -= constraint.jacDiagInv * dot(constraint.axis, deltaVelocityA - deltaVelocityB);
			float oldImpulse = constraint.accumImpulse;
			constraint.accumImpulse = glm::clamp(oldImpulse + deltaImpulse, constraint.lowerLimit, constraint.upperLimit);
			deltaImpulse = constraint.accumImpulse - oldImpulse;
			solverBodyA.deltaLinearVelocity += deltaImpulse * solverBodyA.massInv * constraint.axis;
			solverBodyA.deltaAngularVelocity += deltaImpulse * solverBodyA.inertiaInv * cross(rA, constraint.axis);
			solverBodyB.deltaLinearVelocity -= deltaImpulse * solverBodyB.massInv * constraint.axis;
			solverBodyB.deltaAngularVelocity -= deltaImpulse * solverBodyB.inertiaInv * cross(rB, constraint.axis);
		}

		// 衝突の拘束の計算
		for (SpxUInt32 i = 0; i < numPairs; i++)
		{
			const SpxPair& pair = pairs[i];

			SpxSolverBody& solverBodyA = solverBodies[pair.rigidBodyA];
			SpxSolverBody& solverBodyB = solverBodies[pair.rigidBodyB];

			for (SpxUInt32 j = 0; j < pair.contact->m_numContacts; j++)
			{
				SpxContactPoint& cp = pair.contact->m_contactPoints[j];
				glm::vec3 rA = solverBodyA.orientation * cp.pointA;
				glm::vec3 rB = solverBodyB.orientation * cp.pointB;

				// 衝突法線ベクトル方向の拘束
				{
					SpxConstraint& constraint = cp.constraints[0];
					float deltaImpulse = constraint.rhs;
					glm::vec3 deltaVelocityA = solverBodyA.deltaLinearVelocity + cross(solverBodyA.deltaAngularVelocity, rA);
					glm::vec3 deltaVelocityB = solverBodyB.deltaLinearVelocity + cross(solverBodyB.deltaAngularVelocity, rB);
					// 拘束力の計算
					deltaImpulse -= constraint.jacDiagInv * dot(constraint.axis, deltaVelocityA - deltaVelocityB);
					float oldImpulse = constraint.accumImpulse;
					constraint.accumImpulse = glm::clamp(oldImpulse + deltaImpulse, constraint.lowerLimit, constraint.upperLimit);
					deltaImpulse = constraint.accumImpulse - oldImpulse;

					// 求めた拘束力から並進速度、回転速度を更新
					solverBodyA.deltaLinearVelocity += deltaImpulse * solverBodyA.massInv * constraint.axis;
					solverBodyA.deltaAngularVelocity += deltaImpulse * solverBodyA.inertiaInv * cross(rA, constraint.axis);
					solverBodyB.deltaLinearVelocity -= deltaImpulse * solverBodyB.massInv * constraint.axis;
					solverBodyB.deltaAngularVelocity -= deltaImpulse * solverBodyB.inertiaInv * cross(rB, constraint.axis);
				}

				// 反発方向の拘束力が求まったら、摩擦方向の拘束力の最大値と最小値が決定できるので、これらを計算する。
				// 摩擦力の最大値は (動)摩擦係数 * 垂直抗力
				float maxFriction = pair.contact->m_friction * glm::abs(cp.constraints[0].accumImpulse);
				cp.constraints[1].lowerLimit = -maxFriction;
				cp.constraints[1].upperLimit = maxFriction;
				cp.constraints[2].lowerLimit = -maxFriction;
				cp.constraints[2].upperLimit = maxFriction;

				// 摩擦方向の拘束その1
				{
					SpxConstraint& constraint = cp.constraints[1];
					float deltaImpulse = constraint.rhs;
					glm::vec3 deltaVelocityA = solverBodyA.deltaLinearVelocity + cross(solverBodyA.deltaAngularVelocity, rA);
					glm::vec3 deltaVelocityB = solverBodyB.deltaLinearVelocity + cross(solverBodyB.deltaAngularVelocity, rB);
					deltaImpulse -= constraint.jacDiagInv * dot(constraint.axis, deltaVelocityA - deltaVelocityB);
					float oldImpulse = constraint.accumImpulse;
					constraint.accumImpulse = glm::clamp(oldImpulse + deltaImpulse, constraint.lowerLimit, constraint.upperLimit);
					deltaImpulse = constraint.accumImpulse - oldImpulse;
					solverBodyA.deltaLinearVelocity += deltaImpulse * solverBodyA.massInv * constraint.axis;
					solverBodyA.deltaAngularVelocity += deltaImpulse * solverBodyA.inertiaInv * cross(rA, constraint.axis);
					solverBodyB.deltaLinearVelocity -= deltaImpulse * solverBodyB.massInv * constraint.axis;
					solverBodyB.deltaAngularVelocity -= deltaImpulse * solverBodyB.inertiaInv * cross(rB, constraint.axis);
				}

				// 摩擦方向の拘束その2
				{
					SpxConstraint& constraint = cp.constraints[2];
					float deltaImpulse = constraint.rhs;
					glm::vec3 deltaVelocityA = solverBodyA.deltaLinearVelocity + cross(solverBodyA.deltaAngularVelocity, rA);
					glm::vec3 deltaVelocityB = solverBodyB.deltaLinearVelocity + cross(solverBodyB.deltaAngularVelocity, rB);
					deltaImpulse -= constraint.jacDiagInv * dot(constraint.axis, deltaVelocityA - deltaVelocityB);
					float oldImpulse = constraint.accumImpulse;
					constraint.accumImpulse = glm::clamp(oldImpulse + deltaImpulse, constraint.lowerLimit, constraint.upperLimit);
					deltaImpulse = constraint.accumImpulse - oldImpulse;
					solverBodyA.deltaLinearVelocity += deltaImpulse * solverBodyA.massInv * constraint.axis;
					solverBodyA.deltaAngularVelocity += deltaImpulse * solverBodyA.inertiaInv * cross(rA, constraint.axis);
					solverBodyB.deltaLinearVelocity -= deltaImpulse * solverBodyB.massInv * constraint.axis;
					solverBodyB.deltaAngularVelocity -= deltaImpulse * solverBodyB.inertiaInv * cross(rB, constraint.axis);
				}
			}
		}
	}

	// 拘束力から算出された速度の差分を各剛体の速度に加える
	for (SpxUInt32 i = 0; i < numRigidBodies; i++)
	{
		states[i].m_linearVelocity += solverBodies[i].deltaLinearVelocity;
		states[i].m_angularVelocity += solverBodies[i].deltaAngularVelocity;
	}

	allocator->deallocate(solverBodies);
}

};	// namespace SimplePhysics