// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Units/RigUnit.h"
#include "RigUnit_LookAtMath.generated.h"

/**
 * Returns the normalized normal of the plane defined by three points.
 * The normal is computed as (B - A) x (C - A), so swapping B and C flips it.
 * If the points are collinear or coincident, Normal is zero and bIsValid is false.
 */
USTRUCT(meta=(DisplayName="Plane Normal From Points", Category="Math|Vector", Keywords="Triangle,Cross,Normal,Plane", NodeColor="0.05 0.25 0.05"))
struct LOOKATDEMO_API FRigUnit_PlaneNormalFromPoints : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_PlaneNormalFromPoints()
	{
		A = B = C = Normal = FVector::ZeroVector;
		bIsValid = false;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta=(Input))
	FVector A;

	UPROPERTY(meta=(Input))
	FVector B;

	UPROPERTY(meta=(Input))
	FVector C;

	UPROPERTY(meta=(Output))
	FVector Normal;

	UPROPERTY(meta=(Output))
	bool bIsValid;
};

/**
 * Returns the signed angle (in radians, in [-PI, PI]) to rotate Point1 around Axis so it lines up with Point2.
 * The rotation axis passes through the origin, matching FQuat(Axis, Angle).RotateVector(Point1).
 * If the points are at different distances from the axis, the rotated point is aligned with Point2
 * (same direction around the axis) rather than equal to it.
 * If either point lies on the axis, or the axis is zero, Angle is 0 and bIsValid is false.
 */
USTRUCT(meta=(DisplayName="Angle Around Axis Between Points", Category="Math|Vector", Keywords="Angle,Axis,Rotate,Quaternion,Twist", NodeColor="0.05 0.25 0.05"))
struct LOOKATDEMO_API FRigUnit_AngleAroundAxisBetweenPoints : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_AngleAroundAxisBetweenPoints()
	{
		Axis = FVector::ZAxisVector;
		Point1 = Point2 = FVector::ZeroVector;
		Angle = 0.f;
		bIsValid = false;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta=(Input))
	FVector Axis;

	UPROPERTY(meta=(Input))
	FVector Point1;

	UPROPERTY(meta=(Input))
	FVector Point2;

	UPROPERTY(meta=(Output))
	float Angle;

	UPROPERTY(meta=(Output))
	bool bIsValid;
};

/**
 * Returns the distance between a point and the infinite line going through the origin along Direction.
 * If Direction is zero, Distance is the distance to the origin and bIsValid is false.
 */
USTRUCT(meta=(DisplayName="Distance To Line Through Origin", Category="Math|Vector", Keywords="Distance,Line,Axis,Radius", NodeColor="0.05 0.25 0.05"))
struct LOOKATDEMO_API FRigUnit_DistanceToLineThroughOrigin : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_DistanceToLineThroughOrigin()
	{
		Direction = FVector::ZAxisVector;
		Point = FVector::ZeroVector;
		Distance = 0.f;
		bIsValid = false;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta=(Input))
	FVector Direction;

	UPROPERTY(meta=(Input))
	FVector Point;

	UPROPERTY(meta=(Output))
	float Distance;

	UPROPERTY(meta=(Output))
	bool bIsValid;
};

/**
 * Returns the point on the infinite line going through the origin along Direction that is closest to Point.
 * If Direction is zero, ClosestPoint is the origin and bIsValid is false.
 */
USTRUCT(meta=(DisplayName="Closest Point On Line Through Origin", Category="Math|Vector", Keywords="Closest,Project,Line,Axis,Nearest", NodeColor="0.05 0.25 0.05"))
struct LOOKATDEMO_API FRigUnit_ClosestPointOnLineThroughOrigin : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_ClosestPointOnLineThroughOrigin()
	{
		Direction = FVector::ZAxisVector;
		Point = ClosestPoint = FVector::ZeroVector;
		bIsValid = false;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta=(Input))
	FVector Direction;

	UPROPERTY(meta=(Input))
	FVector Point;

	UPROPERTY(meta=(Output))
	FVector ClosestPoint;

	UPROPERTY(meta=(Output))
	bool bIsValid;
};

/**
 * Decomposes a quaternion into swing and twist around TwistAxis (up by default) and inverts it
 * when the sign of the twist along TwistAxis doesn't match bPositiveSwing.
 * Result is either Value or Inverse(Value), which is the opposite rotation (not the same rotation like -Value).
 * Swing and Twist are the decomposition of Result.
 */
USTRUCT(meta=(DisplayName="Quaternion Conditional Inverse", Category="Math|Quaternion", Keywords="Swing,Twist,Flip,Inverse,Sign", NodeColor="0.05 0.25 0.05"))
struct LOOKATDEMO_API FRigUnit_QuaternionSwingSign : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_QuaternionSwingSign()
	{
		Value = Result = FQuat::Identity;
		TwistAxis = FVector::ZAxisVector;
		bPositiveSwing = true;
		bFlipped = false;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta=(Input))
	FQuat Value;

	UPROPERTY(meta=(Input))
	FVector TwistAxis;

	/** The quaternion is inverted when the twist along TwistAxis is negative (if true) or positive (if false) */
	UPROPERTY(meta=(Input))
	bool bPositiveSwing;

	UPROPERTY(meta=(Output))
	FQuat Result;

	/** True if Result is the inverted input */
	UPROPERTY(meta=(Output))
	bool bFlipped;
};

USTRUCT(meta=(DisplayName="Quaternion Break", Category="Math|Quaternion", Keywords="Quat, Quaternion, Break, Split", NodeColor="0.05 0.25 0.05"))
struct LOOKATDEMO_API FRigUnit_QuaternionBreak : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_QuaternionBreak()
	{
		Value = FQuat::Identity;
		W = X = Y = Z = 0;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta=(Input))
	FQuat Value;

	UPROPERTY(meta=(Output))
	float W;
	
	UPROPERTY(meta=(Output))
	float X;
	
	UPROPERTY(meta=(Output))
	float Y;

	UPROPERTY(meta=(Output))
	float Z;
};


/**
 * Blends the global rotation of Bone towards Target by Alpha, keeping the blend continuous over time.
 * The rotation from the bone to Target (Delta) can go the short or the long way. The way is kept continuous
 * with the previous frame by keeping the sign of Delta continuous, so the blend doesn't flip when Target crosses 180 degrees.
 * The previous Delta is stored as metadata on the bone itself (named HistoryName), so the same node can be
 * used for several bones in loops and functions. Hidden node state isn't used because RigVM slices it per loop iteration.
 * Reset (or the first evaluation of a bone) always uses the short path.
 */
USTRUCT(meta=(DisplayName="Continuous Slerp", Category="Math|Quaternion", Keywords="Slerp,Blend,Lerp,Continuous,Flip,LongPath,ShortPath", NodeColor="0.05 0.25 0.05", Varying))
struct LOOKATDEMO_API FRigUnit_ContinuousSlerp : public FRigUnit
{
	GENERATED_BODY()

	FRigUnit_ContinuousSlerp()
	{
		Bone = FRigElementKey(NAME_None, ERigElementType::Bone);
		Target = Result = FQuat::Identity;
		Alpha = 0.f;
		bReset = false;
		HistoryName = TEXT("ContinuousSlerpPrevious");
		bLongPath = false;
		TotalAngle = 0.f;
	}

	RIGVM_METHOD()
	virtual void Execute() override;

	/** The bone whose global rotation is blended, also used to store its previous result */
	UPROPERTY(meta=(Input, ExpandByDefault))
	FRigElementKey Bone;

	/** The global rotation to blend towards */
	UPROPERTY(meta=(Input))
	FQuat Target;

	UPROPERTY(meta=(Input))
	float Alpha;

	/** Forces the short path and restarts the history of this bone */
	UPROPERTY(meta=(Input))
	bool bReset;

	/** Name of the metadata storing the previous result on the bone, only change it if two of these nodes drive the same bone */
	UPROPERTY(meta=(Input))
	FName HistoryName;

	/** The blended global rotation */
	UPROPERTY(meta=(Output))
	FQuat Result;

	/** True if the long path was used */
	UPROPERTY(meta=(Output))
	bool bLongPath;

	/** Angle in degrees from the bone to Target along the chosen way (above 180 means the long way) */
	UPROPERTY(meta=(Output))
	float TotalAngle;

	UPROPERTY()
	FCachedRigElement CachedBone;
};
