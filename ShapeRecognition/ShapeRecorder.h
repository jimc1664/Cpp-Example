// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//~~~~~~~~~~~~ UMG ~~~~~~~~~~~~~~~~
//#include "Runtime/UMG/Public/UMG.h"
//#include "Runtime/UMG/Public/UMGStyle.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#include "GameFramework/Actor.h"
#include "Shape.h"  //todo - needed?
#include "ShapeRecorder.generated.h"


UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EDebugShapeMatch : uint8 {
	None 	UMETA(DisplayName = "None"),
	LineTo 	UMETA(DisplayName = "LineTo"),
	Overlay	UMETA(DisplayName = "Overlay"),
	Single	UMETA(DisplayName = "Single"),
	OverlayEach	UMETA(DisplayName = "OverlayEach")
};


class UBaseShapeRec;

UCLASS()
class MINT_API AShapeRecorder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShapeRecorder();
	~AShapeRecorder();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
	//TWeakObjectPtr<AShape> Shp;
	
	UPROPERTY(EditAnywhere)
		TSubclassOf<AShape> ShapeType;

	UPROPERTY(EditAnywhere)
		TArray<TWeakObjectPtr<AShape>> Shapes;

	UPROPERTY(EditAnywhere)
		EDebugShapeMatch DebugShapeMatch;

	UPROPERTY(EditAnywhere)
		FString SaveFn;

	UPROPERTY(EditAnywhere)
		bool SaveToFile;

	UPROPERTY(EditAnywhere)
		bool LoadFromFile;

	UPROPERTY(EditAnywhere)
		bool Clear;

	UPROPERTY(EditAnywhere)
		bool AutoSave;

	UPROPERTY(EditAnywhere)
		bool ReportLast;
	UPROPERTY(EditAnywhere)
		bool ReportAll;

	UPROPERTY(EditAnywhere)
		bool EvalShadow;

	UPROPERTY(EditAnywhere)
		bool UseOld;
	UPROPERTY(EditAnywhere)
		bool Eval2;

	UPROPERTY(EditAnywhere)
		bool ForceUp;

	UPROPERTY(Category = UserInterface, EditAnywhere, BlueprintReadWrite)
		FString DebugTxt;


	void add(AShape &s);

	AShape* eval( AShape & o);

	static EShapeLabel eval( UBaseShapeRec &shp );

	AShape * LastEval;
	AShape * LastPicked;

	int SinglePickI;
	AShape * SinglePick;
private:
	void keyProc(const FKey &k, const EShapeCaptureSrc &scs, const APlayerController * playerController);


//	UTextBlock * DebugTxt;

};
