// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Shape.h"
#include "GameFramework/SaveGame.h"
#include "ShapeSave.generated.h"

/**
 * 
 */

class AShape;

USTRUCT()
struct FShapeSaveDat {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FSrcPoint> SrcPoints;
	UPROPERTY(EditAnywhere)
	FBox SrcBnds;  

	UPROPERTY(EditAnywhere)
		FName Name;
		
		
	UPROPERTY(EditAnywhere)
		EShapeLabel ShapeType;

	UPROPERTY(EditAnywhere)
		float Threshold;

	void init(AShape &s);
};

UCLASS()
class MINT_API UShapeSave : public USaveGame
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere)
	TArray<FShapeSaveDat> Shapes;

	void init(const TArray<TWeakObjectPtr<AShape>> &shapes);

};
