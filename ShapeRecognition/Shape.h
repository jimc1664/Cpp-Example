// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Shape.generated.h"

class AShapeRecorder; class UBaseShapeRec;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EShapeLabel : uint8 {
	Invalid UMETA(DisplayName = "Invalid"),
	Fire UMETA(DisplayName = "Fire"),
	Water UMETA(DisplayName = "Water"),
	Electricity UMETA(DisplayName = "Electricity"),
	Earth UMETA(DisplayName = "Earth"),
	Projectile UMETA(DisplayName = "Projectile"),
	Summon UMETA(DisplayName = "Summon"),
	Trap UMETA(DisplayName = "Trap"),
	Wall UMETA(DisplayName = "Wall"),
	Creature UMETA(DisplayName = "Creature"),
	Emit UMETA(DisplayName = "Emit"),
	Energy UMETA(DisplayName = "Energy"),
	Imbue UMETA(DisplayName = "Imbue"),
	Stone UMETA(DisplayName = "Stone"),
	Trow UMETA(DisplayName = "Trow"),
	Area UMETA(DisplayName = "Area"),
	Hand UMETA(DisplayName = "Hand"),
	Barrier UMETA(DisplayName = "Barrier"),
	Entity UMETA(DisplayName = "Entity"),
	Jump UMETA(DisplayName = "Jump"),
	Slow UMETA(DisplayName = "Slow"),
	Totem UMETA(DisplayName = "Totem"),
	Pull UMETA(DisplayName = "Pull"),
	Weapon UMETA(DisplayName = "Weapon"),
	Strengthen UMETA(DisplayName = "Strengthen"),
	Ignite UMETA(DisplayName = "Ignite"),
	Stream UMETA(DisplayName = "Stream"),
	Follow UMETA(DisplayName = "Follow")
};

// Dan was here
USTRUCT(BlueprintType)
struct FSpellCombination {
	GENERATED_USTRUCT_BODY()

	FSpellCombination() {}

	UPROPERTY(EditAnywhere)
		TArray<EShapeLabel> shapes;
	UPROPERTY(EditAnywhere)
		int spellId;
};

enum class EShapeCaptureSrc : uint8 {
	Mouse,
	LeftController,
	RightController,
}


USTRUCT(BlueprintType)
struct FSrcPoint {
	GENERATED_USTRUCT_BODY()
		FSrcPoint() {}
		FSrcPoint(const FVector &p, const FVector &d) : P(p), CtrlrD(d) {}

	UPROPERTY(EditAnywhere)
		FVector P;
	UPROPERTY(EditAnywhere)
		FVector CtrlrD;
};

UCLASS()
class MINT_API AShape : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShape();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void init_Drawn(const FKey &k, const EShapeCaptureSrc &sc, AShapeRecorder *rec);
	void init(const TArray<FSrcPoint> &sp,  const FBox &sb, AShapeRecorder *rec);
	void init( UBaseShapeRec & src, AShapeRecorder *rec);


	float evaluate(const AShape &other, bool drawLineTo, bool drawOverLay); //const;

	//! don't use this one
	float evaluate2(const AShape &other, bool drawLineTo, bool drawOverLay); //const;


	UPROPERTY(EditAnywhere)
		EShapeLabel ShapeType;
		
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> EffectBP;

	UPROPERTY(EditAnywhere)
		float LastEvalVal;
	UPROPERTY(EditAnywhere)
		float NormScale;
	UPROPERTY(EditAnywhere)
		int NPointCount;
	UPROPERTY(EditAnywhere)
		FVector HandOffset;
	UPROPERTY(EditAnywhere)
		float DrawDelta;
	UPROPERTY(EditAnywhere)
		float NormDelta;	
	UPROPERTY(EditAnywhere)
		float DirFactor;
	UPROPERTY(EditAnywhere)
		int ShuffleIter;
	UPROPERTY(EditAnywhere)
		float Threshold;
	UPROPERTY(EditAnywhere)
		float MouseProjection;

	UPROPERTY(EditAnywhere)
		bool DrawLines;
	UPROPERTY(EditAnywhere)
		bool DrawNormLines;

	UPROPERTY(EditAnywhere)
		bool Rebuild;


	UPROPERTY(EditAnywhere)
		bool Report;

	AShape *EvalPick;

	struct Point {
		Point(const FVector &p) : P(p), D(FVector::ZeroVector) {}
		Point(const FVector &p, const FVector &d) : P(p), D(d) {}
		FVector P, D;
	};
private: friend struct FShapeSaveDat;


	bool getControllerPosition(APlayerController * playerController, FVector &mouseLocation, FVector &mouseDirection);
	float pDev(const Point &a, const Point &b) const;
	float pDev2(const Point &a, const Point &b) const;

	FVector normalisePoint(const FVector &p) const;
	FVector deNormalisePoint(const FVector &p) const;

	void drawShape() const;
	void drawShadow( const AShape &other ) const;

	void build();

	FKey DrawKey;
	bool Drawing;
	EShapeCaptureSrc SCs;

	AShapeRecorder *Recorder;

	AActor* Effect;

	TArray<FSrcPoint> SrcPoints;
	FBox SrcBnds;  //just needed to render debug lines
	FVector AvgDir;

	struct Mat3x3 {
		Mat3x3() {
			x = FVector::ForwardVector;
			z = FVector::UpVector;
			y = FVector::RightVector;
		}
		FVector x, y, z;

		FVector mul(const FVector &v) const {
			return
				x * v.X +
				y * v.Y +
				z * v.Z;
		}		
		FVector mulTranspose(const FVector &v) const {
			return
				FVector(FVector::DotProduct(v, x), FVector::DotProduct(v, y), FVector::DotProduct(v, z));
		}
	};
	Mat3x3 SrcRot;

	TArray<Point> Points;
		
	void BeginDestroy() override;
	

	int MsgKey;
};
