#include "pch.h"
#include "BoxCollider2D.h"
#include <numbers>

PhysicsArchive physicsArchive = PhysicsArchive();

 BoxCollider2D BoxCollider2D_CreateBoxCollider2D(vec2& min, vec2& max)
{
	 vec2 size = max - min;
	 return BoxCollider2D
	 {
		 .Size = size,
		 .HalfSize = min + (vec2(size) * 0.5f)
	 };
 }

 vec2 BoxCollider2D_GetBoxCollider2DMin(BoxCollider2D& boxCollider, RigidBody& rigidBody)
 {
	 return rigidBody.position - boxCollider.HalfSize;
 }

 vec2 BoxCollider2D_GetBoxCollider2DMax(BoxCollider2D& boxCollider, RigidBody& rigidBody)
 {
	 return rigidBody.position + boxCollider.HalfSize;
 }

 Vector<vec2> BoxCollider2D_GetBoxColliderVertices(BoxCollider2D& boxCollider, RigidBody& rigidBody)
 {
	 const vec2& min = BoxCollider2D_GetBoxCollider2DMin(boxCollider, rigidBody);
	 const vec2& max = BoxCollider2D_GetBoxCollider2DMax(boxCollider, rigidBody);
	 
	 Vector<vec2> colliderVertices
	 {
		vec2(min.x, min.y),
		vec2(min.x, max.y),
		vec2(max.x, min.y),
		vec2(max.x, max.y)
	 };

	 if (rigidBody.rotation != 0.0f)
	 {
		 const vec2 center = rigidBody.position;
		 const float thetaRad = rigidBody.rotation * (std::numbers::pi / 180);
		 const float cosTheta = cosf(thetaRad);
		 const float sinTheta = sinf(thetaRad);
		 for (auto& vertex : colliderVertices)
		 {
			 const float relativeX = vertex.x - center.x;
			 const float relativeY = vertex.y - center.y;
			 const float rotationX = relativeX * cosTheta - relativeY * sinTheta;
			 const float rotationY = relativeX * sinTheta - relativeY * cosTheta;

			 vertex.x = rotationX + center.x;
			 vertex.y = rotationY + center.y;
		 }
	 }

	 return colliderVertices;
 }
