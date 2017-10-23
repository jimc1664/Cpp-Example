// Fill out your copyright notice in the Description page of Project Settings.

#include "Mint.h"
#include "ShapeRecorder.h"

#include "ShapeSave.h"
#include "Kismet/GameplayStatics.h"


AShapeRecorder *Singleton = nullptr;
// Sets default values
AShapeRecorder::AShapeRecorder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//UWidget * tmp;
	
//	Widget->GetComponentByClass<AW
	//should be null...
	Singleton = this;
}

AShapeRecorder::~AShapeRecorder()
{
	
	if (Singleton == this)
		Singleton = nullptr;

}

// Called when the game starts or when spawned
void AShapeRecorder::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShapeRecorder::keyProc(const FKey &k, const EShapeCaptureSrc &scs, const APlayerController * playerController)
{
	if (playerController->WasInputKeyJustPressed(k)) {

		auto plyrPwn = playerController->GetPawn();

		//auto shp = GetWorld()->SpawnActor<AShape>(plyrPwn->GetActorLocation(), plyrPwn->GetActorRotation());

		auto act = GetWorld()->SpawnActor(ShapeType);
		auto shp = dynamic_cast<AShape*>(act);
		if (!shp) {
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn  ?? %d  --- %d  "), (size_t)act, (size_t)shp);
		} else {
			shp->init_Drawn(k, scs, this );

			//TSubobjectPtr<USceneComponent> SceneComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
			//RootComponent = SceneComponent;

			UE_LOG(LogTemp, Warning, TEXT("Spawn4 ??  "));
			//Shp = GetWorld()->SpawnActor( AShape::StaticClass(), NAME_None, NULL, NULL, NULL, false, false, NULL, NULL, NULL, false );
		}
	}
}


// Called every frame
void AShapeRecorder::Tick( float DeltaTime )
{

	Super::Tick( DeltaTime );
	try {
	//	FVector mouseLocation, mouseDirection;
		APlayerController * playerController = GetWorld()->GetFirstPlayerController();
	//	playerController->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);
		
		if (UseOld) {
			if (!playerController->IsInputKeyDown(EKeys::LeftControl)) {
				keyProc(EKeys::LeftMouseButton, EShapeCaptureSrc::Mouse, playerController);
				keyProc(EKeys::RightMouseButton, EShapeCaptureSrc::Mouse, playerController);

			}
			if (!playerController->IsInputKeyDown(EKeys::MotionController_Left_Grip1)) {
				keyProc(EKeys::MotionController_Left_Trigger, EShapeCaptureSrc::LeftController, playerController);
			}

			if (!playerController->IsInputKeyDown(EKeys::MotionController_Right_Grip1)) {
				keyProc(EKeys::MotionController_Right_Trigger, EShapeCaptureSrc::RightController, playerController);
			}
		}
		if(DebugShapeMatch == EDebugShapeMatch::Single ) {
			
			if (playerController->WasInputKeyJustReleased(EKeys::MotionController_Right_Thumbstick) && Shapes.Num() > 0 ) {

				playerController->InputComponent->BindAxisKey(EKeys::MotionController_Right_Thumbstick_X);
				auto axX = playerController->GetInputAxisKeyValue(EKeys::MotionController_Right_Thumbstick_X);
				float ep = 0.25f;
				if( axX > ep) {
					if (++SinglePickI >= Shapes.Num())
						SinglePickI = 0;
					SinglePick = &*Shapes[SinglePickI];
				} else if(axX < -ep) {
					if (--SinglePickI < 0)
						SinglePickI = Shapes.Num()-1;
					SinglePick = &*Shapes[SinglePickI];
				}
			}
		}
			 
		if (SaveToFile) {
			SaveToFile = false;
			UShapeSave* saveGameInstance = Cast<UShapeSave>(UGameplayStatics::CreateSaveGameObject(UShapeSave::StaticClass()));
			saveGameInstance->init(Shapes);
			UGameplayStatics::SaveGameToSlot(saveGameInstance, SaveFn, 0);
		}

		if (LoadFromFile) {
			LoadFromFile = false;

			UShapeSave* sg = Cast<UShapeSave>(UGameplayStatics::CreateSaveGameObject(UShapeSave::StaticClass()));
			sg = Cast<UShapeSave>(UGameplayStatics::LoadGameFromSlot(SaveFn, 0) );
			if( sg )
				for( auto s : sg->Shapes )
				{
					//FActorSpawnParameters asp;
					//asp.Name = s.Name;
					UE_LOG(LogTemp, Warning, TEXT("to spawn %s "), *(s.Name.ToString()) );
					//continue;
					FActorSpawnParameters asp; asp.Name = s.Name;
					auto act = GetWorld()->SpawnActor<AShape>(ShapeType, asp );
					auto shp = dynamic_cast<AShape*>(act);
					if (!shp) {
						UE_LOG(LogTemp, Warning, TEXT("Failed to spawn  ?? %d  --- %d  "), (size_t)act, (size_t)shp);
					} else {
						//shp->ClearActorLabel();
						shp->SetActorLabel( s.Name.ToString() );
						shp->init( s.SrcPoints, s.SrcBnds, this);
						shp->ShapeType = s.ShapeType;
						shp->Threshold = s.Threshold;
						//shp->init_Drawn( EKeys::LeftMouseButton, EShapeCaptureSrc::Mouse, this);
						add(*shp);
					}
				}
		}		
		
		if (Clear) {
			Clear = false;
			

			for (auto o : Shapes) {
				o->Destroy();
			}
			Shapes.Empty();
		}
	}
	catch (const std::exception& e) {
		UE_LOG(LogTemp, Warning, TEXT("AShapeRecorder::Tick :- Some error : %s"), e.what());
	}
	catch (...) {
		UE_LOG(LogTemp, Warning, TEXT("AShapeRecorder::Tick :-Weird Error"));
	}
}

AShape* AShapeRecorder::eval( AShape & shp )
{
	LastEval = &shp;
	auto lp = LastPicked;
	AShape* ret = LastPicked = nullptr;
	float cd = 99999999;
	float cd2 = 99999999;


	if (ReportAll || ReportLast) {
		DebugTxt = "Evaluating  " + shp.GetName() +"\n";
	}
	for( auto o : Shapes )
	{
		auto d = Eval2 ? shp.evaluate2(*o, lp == &*o && DebugShapeMatch == EDebugShapeMatch::LineTo, lp == &*o && DebugShapeMatch == EDebugShapeMatch::Overlay )
			: shp.evaluate(*o, lp == &*o && DebugShapeMatch == EDebugShapeMatch::LineTo, lp == &*o && DebugShapeMatch == EDebugShapeMatch::Overlay);

		UE_LOG(LogTemp, Warning, TEXT("eval   ?? %s  --- %f "), *(o->GetName()), d);
		if (ReportAll) {
			DebugTxt += o->GetName()  +"   val (threshold):  "+ FString::SanitizeFloat(d)  + " ("+ FString::SanitizeFloat(o->Threshold) + ")\n";
		}

		if( d  < cd )
		{
			LastPicked = &*o;
			cd = d;		
		}
		
		if( d < o->Threshold && d < cd2 ) {
			ret = &*o;
			cd2 = d;
		}

	}
	shp.EvalPick = LastPicked;

	if( ReportLast ) {
		FString msg = " Last eval :- ";

		if (ret) {
			msg += ret->GetName();
			msg += "   val = ";
			msg += FString::SanitizeFloat(cd2);
		} else
		{
			msg += "none found";
		}
		DebugTxt += "-----------------\n"+ msg;
		GEngine->bEnableOnScreenDebugMessages = true;
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Purple, msg);

		//UKismetSystemLibrary::PrintString(GetWorld(), (FString)"fuck you" + FString::SanitizeFloat( FLT_MAX)  );
	}
	return ret;
}

EShapeLabel AShapeRecorder::eval(UBaseShapeRec & src) {

	if( Singleton != nullptr && !Singleton->UseOld)
	{
		UE_LOG(LogTemp, Warning, TEXT("temp spawn  ") );
		//continue;
		auto act = Singleton->GetWorld()->SpawnActor(Singleton->ShapeType);
		auto shp = dynamic_cast<AShape*>(act);
		if (!shp) {
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn  ?? %d  --- %d  "), (size_t)act, (size_t)shp);
		} else {
			shp->init( src, Singleton);
			//shp->init_Drawn( EKeys::LeftMouseButton, EShapeCaptureSrc::Mouse, this);
			
			auto pick = Singleton->eval( *shp );
			Singleton->LastEval = nullptr;
			shp->Destroy();
			if( pick )
				return pick->ShapeType;
		}
	}
	return EShapeLabel::Invalid;
}

void AShapeRecorder::add(AShape &s)
{
	
	Shapes.Add(TWeakObjectPtr<AShape>(&s));
	if (AutoSave) {
		UShapeSave* saveGameInstance = Cast<UShapeSave>(UGameplayStatics::CreateSaveGameObject(UShapeSave::StaticClass()));
		saveGameInstance->init(Shapes);
		UGameplayStatics::SaveGameToSlot(saveGameInstance, SaveFn, 0);
	}

}