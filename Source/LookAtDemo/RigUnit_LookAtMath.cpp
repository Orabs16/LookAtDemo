// Copyright Epic Games, Inc. All Rights Reserved.

#include "RigUnit_LookAtMath.h"
#include "Units/RigUnitContext.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(RigUnit_LookAtMath)

FRigUnit_PlaneNormalFromPoints_Execute()
{
	const FVector Cross = (B - A) ^ (C - A);
	bIsValid = !Cross.IsNearlyZero(UE_SMALL_NUMBER);
	Normal = bIsValid ? Cross.GetUnsafeNormal() : FVector::ZeroVector;
}

FRigUnit_AngleAroundAxisBetweenPoints_Execute()
{
	Angle = 0.f;
	bIsValid = false;

	const FVector UnitAxis = Axis.GetSafeNormal();
	if (UnitAxis.IsZero())
	{
		return;
	}

	// Project both points onto the plane perpendicular to the axis
	const FVector V1 = Point1 - (Point1 | UnitAxis) * UnitAxis;
	const FVector V2 = Point2 - (Point2 | UnitAxis) * UnitAxis;
	if (V1.IsNearlyZero(UE_KINDA_SMALL_NUMBER) || V2.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
	{
		return;
	}

	// Signed angle, positive following the same convention as FQuat(UnitAxis, Angle)
	Angle = FMath::Atan2(UnitAxis | (V1 ^ V2), V1 | V2);
	bIsValid = true;
}

FRigUnit_DistanceToLineThroughOrigin_Execute()
{
	const FVector UnitDirection = Direction.GetSafeNormal();
	bIsValid = !UnitDirection.IsZero();
	Distance = bIsValid ? (Point ^ UnitDirection).Size() : Point.Size();
}

FRigUnit_ClosestPointOnLineThroughOrigin_Execute()
{
	const FVector UnitDirection = Direction.GetSafeNormal();
	bIsValid = !UnitDirection.IsZero();
	ClosestPoint = (Point | UnitDirection) * UnitDirection;
}

FRigUnit_QuaternionSwingSign_Execute()
{
	const FVector UnitAxis = TwistAxis.GetSafeNormal(UE_SMALL_NUMBER, FVector::ZAxisVector);
	const FQuat Quat = Value.GetNormalized();
	FQuat Swing;
	FQuat Twist;
	Quat.ToSwingTwist(UnitAxis, Swing, Twist);

	// Twist vector part projected on the axis, works for any axis (equals Twist.Z when the axis is Z)
	const bool bSwingIsPositive = (FVector(Twist.X, Twist.Y, Twist.Z) | UnitAxis) >= 0.f;
	bFlipped = bSwingIsPositive != bPositiveSwing;

	Result = bFlipped ? Quat.Inverse() : Quat;
}

FRigUnit_QuaternionBreak_Execute()
{
	W = Value.W;
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
}

FRigUnit_ContinuousSlerp_Execute()
{
	Result = FQuat::Identity;
	bLongPath = false;

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr)
	{
		return;
	}

	if (!CachedBone.UpdateCache(Bone, Hierarchy))
	{
		UE_CONTROLRIG_RIGUNIT_REPORT_WARNING(TEXT("Bone '%s' is not valid."), *Bone.ToString());
		return;
	}

	const FQuat Base = Hierarchy->GetGlobalTransform(CachedBone).GetRotation().GetNormalized();

	// Rotation left to go from the bone to the target, in bone space.
	// Delta and -Delta are the same rotation, but going the short way (W >= 0) or the long way (W < 0).
	FQuat Delta = (Base.Inverse() * Target.GetNormalized()).GetNormalized();

	// The history lives on the bone, a zero quaternion means there is none yet
	const FQuat NoHistory(0.f, 0.f, 0.f, 0.f);
	const FQuat PreviousDelta = Hierarchy->GetQuatMetadata(Bone, HistoryName, NoHistory);

	if (bReset || PreviousDelta.SizeSquared() < UE_KINDA_SMALL_NUMBER)
	{
		// Short way
		if (Delta.W < 0.f)
		{
			Delta = -Delta;
		}
	}
	else if ((Delta | PreviousDelta) < 0.f)
	{
		// Keep the sign continuous with last frame, which keeps the direction of the blend.
		// Delta and -Delta are opposite in 4D, so this only fails if the rotation changes by 180 degrees in one frame.
		Delta = -Delta;
	}

	Hierarchy->SetQuatMetadata(Bone, HistoryName, Delta);

	bLongPath = Delta.W < 0.f;
	TotalAngle = FMath::RadiansToDegrees(2.f * FMath::Acos(FMath::Clamp(Delta.W, -1.f, 1.f)));

	// Delta^Alpha along the chosen way
	Result = (Base * FQuat::SlerpFullPath(FQuat::Identity, Delta, Alpha)).GetNormalized();
}
