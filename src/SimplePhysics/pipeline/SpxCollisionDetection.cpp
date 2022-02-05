#include "SpxCollisionDetection.h"
#include "../collision/SpxConvexConvexContact.h"
#include "../glmExtension.h"

namespace SimplePhysics
{
void SpxDetectCollision(
	const SpxState* states,
	const SpxCollidable* collidables,
	SpxUInt32 numRigidBodies,
	const SpxPair* pairs,
	SpxUInt32 numPairs)
{
	// 全てのペアに対して調査
	for (SpxUInt32 i = 0; i < numPairs; i++)
	{
		const SpxPair& pair = pairs[i];

		const SpxState& stateA = states[pair.rigidBodyA];
		const SpxState& stateB = states[pair.rigidBodyB];
		const SpxCollidable& collA = collidables[pair.rigidBodyA];
		const SpxCollidable& collB = collidables[pair.rigidBodyB];

		// 3行4列のワールド変換行列を作る
		glm::mat4x3 transformA = GLMExtension::To3x4TransformMat(stateA.m_orientation, stateA.m_position);
		glm::mat4x3 transformB = GLMExtension::To3x4TransformMat(stateB.m_orientation, stateB.m_position);

		// 剛体Aが持つ全ての形状でループ
		for (SpxUInt32 j = 0; j < collA.m_numShapes; j++)
		{
			const SpxShape& shapeA = collA.m_shapes[j];
			glm::mat4x3 offsetTransformA = GLMExtension::To3x4TransformMat(shapeA.m_offsetQuaternion, shapeA.m_offsetPosition);
			glm::mat4x3 worldTransformA = GLMExtension::AffineTransformMultiply(transformA, offsetTransformA);

			// 剛体Bが持つ全ての形状でループ
			for (SpxUInt32 k = 0; k < collB.m_numShapes; k++)
			{
				const SpxShape& shapeB = collB.m_shapes[k];
				glm::mat4x3 offsetTransformB = GLMExtension::To3x4TransformMat(shapeB.m_offsetQuaternion, shapeB.m_offsetPosition);
				glm::mat4x3 worldTransformB = GLMExtension::AffineTransformMultiply(transformB, offsetTransformB);

				glm::vec3 contactPointA;
				glm::vec3 contactPointB;
				glm::vec3 normal;
				float penetrationDepth;

				// 凸メッシュ同士の衝突検出を行う
				if (SpxConvexConvexContact(
						shapeA.m_geometry, worldTransformA,
						shapeB.m_geometry, worldTransformB,
						normal, penetrationDepth,
						contactPointA, contactPointB) &&
					penetrationDepth < 0.0f)
				{
					glm::vec3 contactPointA_localA = GLMExtension::GetTranslation(offsetTransformA) +
													 glm::mat3(offsetTransformA) * contactPointA;
					glm::vec3 contactPointB_localB = GLMExtension::GetTranslation(offsetTransformB) +
													 glm::mat3(offsetTransformB) * contactPointB;

					// 衝突点を剛体の座標系に変換して新しく衝突点として追加する
					pair.contact->AddContact(
						penetrationDepth, normal,
						contactPointA_localA,
						contactPointB_localB);
				}
			}
		}
	}
}
};	// namespace SimplePhysics