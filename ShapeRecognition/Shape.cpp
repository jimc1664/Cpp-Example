// Fill out your copyright notice in the Description page of Project Settings.

#include "Mint.h"
#include "Shape.h"
#include "ShapeRecorder.h"
#include "BaseShapeRec.h"
#include "SteamVRFunctionLibrary.h"

int Glob_MsgKey = 1;
// Sets default values
AShape::AShape() {
	MsgKey = Glob_MsgKey++;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	//RootComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
	SetActorLocation(FVector::ZeroVector);
	DrawKey = EKeys::Invalid;

	ShapeType = EShapeLabel::Invalid;
}

// Called when the game starts or when spawned
void AShape::BeginPlay() {
	Super::BeginPlay();

}


void AShape::init_Drawn(const FKey &k, const EShapeCaptureSrc &sc, AShapeRecorder *rec) {

	Recorder = rec;
	DrawKey = k;
	FVector mouseLocation, mouseDirection;
	SCs = sc;

	APlayerController * playerController = GetWorld()->GetFirstPlayerController();

	Drawing = getControllerPosition(playerController, mouseLocation, mouseDirection);
	if (Drawing) {
		SrcPoints.Add(FSrcPoint(mouseLocation, mouseDirection));

		// Dan was here
		//Effect = GetWorld()->SpawnActor(EffectBP);
		if (Effect)
			Effect->SetActorLocation(mouseLocation);
	}
}
void AShape::init(const TArray<FSrcPoint> &sp, const FBox &sb, AShapeRecorder *rec) {
	Recorder = rec;

	//DrawKey = EKeys;
	SCs = EShapeCaptureSrc::Mouse;
	//SrcPoints = sp;
	SrcPoints.Empty();
	for (auto &s : sp) {
		SrcPoints.Add(s);
	}
	SrcBnds = sb;
	auto cntr = SrcBnds.GetCenter();
	SrcBnds.Min -= cntr;
	SrcBnds.Max -= cntr;

	SetActorLocation(cntr);

	Drawing = false;

	build();
	//Effect = GetWorld()->SpawnActor(EffectBP);
}

void AShape::init(UBaseShapeRec & src, AShapeRecorder *rec) {
	Recorder = rec;

	//DrawKey = EKeys;
	SCs = EShapeCaptureSrc::Mouse;
	//SrcPoints = sp;
	SrcPoints.Empty();
	for (auto &s : src.Points) {
		SrcPoints.Add(s);
	}

	Drawing = false;

	build();
	//Effect = GetWorld()->SpawnActor(EffectBP);
}

// Called every frame
void AShape::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	try {
		/*	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		24,
		32,
		FColor(255, 0, 0)
		); */

		if (DrawKey != EKeys::Invalid) {
			APlayerController * playerController = GetWorld()->GetFirstPlayerController();

#define CLICKTEST( a ) \
if (playerController->WasInputKeyJustPressed( a )) \
	UE_LOG(LogTemp, Warning, TEXT("AShape::tick clicky  " _STRINGIZE(a) ))


			CLICKTEST(EKeys::MotionController_Left_Shoulder);
			CLICKTEST(EKeys::MotionController_Left_Thumbstick);
			CLICKTEST(EKeys::MotionController_Left_Thumbstick_Up);
			CLICKTEST(EKeys::MotionController_Left_Thumbstick_Left);
			CLICKTEST(EKeys::MotionController_Left_Thumbstick_Right);
			CLICKTEST(EKeys::MotionController_Left_Thumbstick_Down);

			if (playerController->IsInputKeyDown(EKeys::MotionController_Left_Shoulder)) {

				UE_LOG(LogTemp, Warning, TEXT("AShape::tick thumb axis (%f, %f) "),
					playerController->GetInputAxisKeyValue(EKeys::MotionController_Left_Thumbstick_X),
					playerController->GetInputAxisKeyValue(EKeys::MotionController_Left_Thumbstick_Y));
			}

			auto key = playerController->IsInputKeyDown(DrawKey);
			if (key) {
				if (Drawing) {
					FVector mouseLocation, mouseDirection;


					if (getControllerPosition(playerController, mouseLocation, mouseDirection)) {

						auto lp = SrcPoints.Last().P;
						if ((lp - mouseLocation).SizeSquared() > DrawDelta*DrawDelta) {
							SrcPoints.Add(FSrcPoint(mouseLocation, mouseDirection));
							//	UE_LOG(LogTemp, Warning, TEXT("AShape::tick :- addP : %f, %f, %f"), mouseLocation.X, mouseLocation.Y, mouseLocation.Z);
							build();

						}

						if (Effect)
							Effect->SetActorLocation(mouseLocation);
					}
				} else {

					if (Effect) {
						Effect->Destroy();
						Effect = 0;
					}
					bool save = false;
					switch (SCs) {
					case EShapeCaptureSrc::Mouse:
						save = playerController->IsInputKeyDown(EKeys::LeftControl);
						//UE_LOG(LogTemp, Warning, TEXT("AShape::getControllerPosition :- MouseP : %f, %f, %f"), mouseLocation.X, mouseLocation.Y, mouseLocation.Z );
						break;
					case EShapeCaptureSrc::LeftController:
						save = playerController->IsInputKeyDown(EKeys::MotionController_Left_Grip1);
						break;
					case EShapeCaptureSrc::RightController:
						save = playerController->IsInputKeyDown(EKeys::MotionController_Right_Grip1);
						break;
					}
					//if (playerController->IsInputKeyDown(EKeys::LeftControl)) {  //save
					if (save) {
						DrawKey = EKeys::Invalid;

						auto cntr = SrcBnds.GetCenter();
						SrcBnds.Min -= cntr;
						SrcBnds.Max -= cntr;

						for (FSrcPoint& p : SrcPoints)
							p.P -= cntr;
						SetActorLocation(cntr);

						Recorder->add(*this);
					} else {
						SrcPoints.Empty();
						Points.Empty();
						Destroy();
						Recorder->LastEval = nullptr;
						return;
					}
				}
			} else
				Drawing = false;

			if (SrcPoints.Num()>2)
				Recorder->eval(*this);
		} else {
			if (Report) {
				FString msg = GetName() + FString::SanitizeFloat(LastEvalVal);
				GEngine->AddOnScreenDebugMessage(MsgKey, 5.f, FColor::Green, msg);
			}
		}
		if (Rebuild) {
			build();
			Rebuild = false;
		}

		if (DrawLines) drawShape();

	}
	catch (const std::exception& e) {
		UE_LOG(LogTemp, Warning, TEXT("AShape::Tick :- Some error : %s"), e.what());
	}
	catch (...) {
		UE_LOG(LogTemp, Warning, TEXT("AShape::Tick :- Weird Error"));
	}
}

bool AShape::getControllerPosition(APlayerController * playerController, FVector &mouseLocation, FVector &mouseDirection) {
	FRotator rot; bool ret;
	switch (SCs) {
	case EShapeCaptureSrc::Mouse:
		ret = playerController->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);

		mouseLocation += mouseDirection *MouseProjection;

		//UE_LOG(LogTemp, Warning, TEXT("AShape::getControllerPosition :- MouseP : %f, %f, %f"), mouseLocation.X, mouseLocation.Y, mouseLocation.Z );
		break;
	case EShapeCaptureSrc::LeftController:
		ret = USteamVRFunctionLibrary::GetHandPositionAndOrientation(0, EControllerHand::Left, mouseLocation, rot);

		mouseLocation = playerController->GetPawn()->ActorToWorld().TransformPosition(mouseLocation);
		//mouseLocation += playerController->GetPawn()->GetActorLocation();
		mouseDirection = rot.Vector();
		mouseLocation += rot.RotateVector(HandOffset);
		//UE_LOG(LogTemp, Warning, TEXT("AShape::getControllerPosition :- CntrlrL : %f, %f, %f"), mouseLocation.X, mouseLocation.Y, mouseLocation.Z);
		break;
	case EShapeCaptureSrc::RightController:
		ret = USteamVRFunctionLibrary::GetHandPositionAndOrientation(0, EControllerHand::Right, mouseLocation, rot);
		mouseLocation = playerController->GetPawn()->ActorToWorld().TransformPosition(mouseLocation);
		//mouseLocation += playerController->GetPawn()->GetActorLocation();
		mouseDirection = rot.Vector();
		{
			auto ho2 = HandOffset;
			ho2.X = -ho2.X;
			mouseLocation += rot.RotateVector(ho2);
		}
		//UE_LOG(LogTemp, Warning, TEXT("AShape::getControllerPosition :- CntrlrR : %f, %f, %f"), mouseLocation.X, mouseLocation.Y, mouseLocation.Z);
		break;
	default: return false;
	}
	return ret;
}

struct PntCmp {
	PntCmp() {
		Off1 = FVector::ZeroVector;
	}
	int Oi;
	FVector Off1, Off2;
};

float AShape::evaluate(const AShape &other, bool drawLineTo, bool drawOverLay) {

	auto wo = GetWorld();

	auto pc = new PntCmp[other.Points.Num()];

	memset(pc, 0, sizeof(pc));
	for (auto i = other.Points.Num(); i-- > 0;)
		pc[i].Oi = i *Points.Num() / other.Points.Num();

	pc[other.Points.Num() - 1].Oi = Points.Num() - 1;


	for (int iter = ShuffleIter; iter-->0; ) {
		for (int i = other.Points.Num() - 1; --i > 0;) {
			auto& i0 = pc[i - 1], i2 = pc[i + 1];
			float cpd = 9999999;
			//int ci = -1;
			for (int i1 = i0.Oi; i1 <= i2.Oi; i1++) {

				float d = pDev(other.Points[i], Points[i1]);
				if (d < cpd) {
					cpd = d;
					pc[i].Oi = i1;
				}
			}
		}
	}

	for (int i = other.Points.Num(); i-- > 0; ) {
		auto vec = other.Points[i].P - Points[pc[i].Oi].P;
		pc[i].Off2 = pc[i].Off1 + vec *0.9f;
	}

	//FVector off = FVector::ZeroVector; int cnt = 0;
	//	int 
	static float mod[] = {
		1,2,3,4,5,6,7,6,5,4,3,2,1
	};

	//float tot = 0; for (int i = ARRAYSIZE(mod); i-- > 0; ) tot += mod[i];
	int spread = 6;
	for (int i = other.Points.Num(); i-- > 0; ) {
		int i1 = i - spread, i2 = i + spread;
		if (i1 < 0) i1 = 0;
		if (i2 >= other.Points.Num()) i2 = other.Points.Num() - 1;

		FVector off = FVector::ZeroVector;
		float tot = 0;
		for (int j = i1; j <= i2; j++) {
			float md = mod[j - i + spread] + 2.0f;
			off += pc[j].Off2 *md;
			tot += md;
		}
		off /= tot;
		pc[i].Off1 = off;
	}

	float dev = 0;
	FVector lp = FVector::ZeroVector;
	bool flag = false;
	for (int i = other.Points.Num(); i-- > 0; ) {
		const auto &p = Points[pc[i].Oi];
		float cpd = pDev(other.Points[i], Point(p.P + pc[i].Off1, p.D));

		if (drawLineTo) {
			auto sp = other.deNormalisePoint(other.Points[i].P) + other.GetActorLocation();
			DrawDebugLine(wo, sp, deNormalisePoint(Points[pc[i].Oi].P) + GetActorLocation(), FColor(0, 0, 255));

		}
		if (drawOverLay) {
			auto sp = deNormalisePoint(other.Points[i].P) + GetActorLocation();
			if (flag) {
				DrawDebugLine(wo, sp, lp, FColor(0, 0, 0));
			} else flag = true;

			auto p1 = deNormalisePoint(Points[pc[i].Oi].P) + GetActorLocation();
			auto pm = deNormalisePoint(Points[pc[i].Oi].P + pc[i].Off1) + GetActorLocation();
			DrawDebugLine(wo, p1, pm, FColor(0, 0, 255));
			DrawDebugLine(wo, pm, sp, FColor(0, 255, 255));
			lp = sp;

		}
		dev += cpd;
	}
	dev /= other.Points.Num();

	delete[] pc;

	LastEvalVal = dev;
	return dev;
}


struct PntDat {

	float D;
};

void gen(const TArray<AShape::Point> &pnts, PntDat *pds) {
	for (int i = pnts.Num(); i-- > 0; ) {


	}
}

float AShape::pDev2(const Point &a, const Point &b) const {


	return 0;
}

float AShape::evaluate2(const AShape &other, bool drawLineTo, bool drawOverLay) {

	auto wo = GetWorld();

	auto pd1 = new PntDat[Points.Num()];  gen(Points, pd1);
	auto pd2 = new PntDat[other.Points.Num()]; gen(other.Points, pd2);

	auto pc = new PntCmp[other.Points.Num()];

	for (auto i = other.Points.Num(); i-- > 0;)
		pc[i].Oi = i *Points.Num() / other.Points.Num();

	pc[other.Points.Num() - 1].Oi = Points.Num() - 1;


	for (int iter = ShuffleIter; iter-->0; ) {
		for (int i = other.Points.Num() - 1; --i > 0;) {
			auto& i0 = pc[i - 1], i2 = pc[i + 1];
			float cpd = FLT_MAX;
			//int ci = -1;
			for (int i1 = i0.Oi; i1 <= i2.Oi; i1++) {

				float d = pDev(other.Points[i], Points[i1]);
				if (d < cpd) {
					cpd = d;
					pc[i].Oi = i1;
				}
			}
		}
	}

	float dev = 0;
	FVector lp = FVector::ZeroVector;
	bool flag = false;
	for (int i = other.Points.Num(); i-- > 0; ) {
		float cpd = pDev(other.Points[i], Points[pc[i].Oi]);

		if (drawLineTo) {
			auto sp = other.deNormalisePoint(other.Points[i].P) + other.GetActorLocation();
			DrawDebugLine(wo, sp, deNormalisePoint(Points[pc[i].Oi].P) + GetActorLocation(), FColor(0, 0, 255));

		}
		if (drawOverLay) {
			auto sp = deNormalisePoint(other.Points[i].P) + GetActorLocation();
			if (flag) {
				DrawDebugLine(wo, sp, lp, FColor(0, 0, 0));
			} else flag = true;

			DrawDebugLine(wo, sp, deNormalisePoint(Points[pc[i].Oi].P) + GetActorLocation(), FColor(0, 0, 255));
			lp = sp;

		}
		dev += cpd;
	}
	dev /= other.Points.Num();

	delete[] pc;
	delete[] pd1;
	delete[] pd2;

	LastEvalVal = dev;
	return dev;
}

void AShape::build() {
	Points.Empty();


	if (SrcPoints.Num() < 2) return;

	FVector mid, mn, mx, dir;   ///mn(FLT_MAX, FLT_MAX, FLT_MAX), mx(FLT_MIN, FLT_MIN, FLT_MIN);  --- wtf

	mid = mn = mx = SrcPoints.Last().P;
	dir = SrcPoints.Last().CtrlrD;
	//foreach(var p in points) {
	for (int i = SrcPoints.Num() - 1; i-- > 0;) {
		const auto& p = SrcPoints[i];// , n = Vector3.zero;

		mid += p.P;
		mn = mn.ComponentMin(p.P);
		mx = mx.ComponentMax(p.P);
		dir += p.CtrlrD;
	}
	mid /= SrcPoints.Num();
	dir.Normalize();

	FVector sz = mx - mn, cntr = (mn + mx)*0.5f;
	cntr = mid;
	sz = mx - mid; sz = sz.ComponentMax(mid - mn);
	sz = FVector(1, 1, 1) * sz.GetMax() * 2;

	mn = cntr - sz*0.5f; mx = cntr + sz *0.5f;

	SrcBnds = FBox(mn, mx);
	AvgDir = dir;

	if (Recorder->ForceUp) {
		SrcRot.y = FVector::CrossProduct(dir, FVector::UpVector);  //right
		SrcRot.x = FVector::CrossProduct(SrcRot.z, SrcRot.y);  //forward
		SrcRot.z = FVector::UpVector;  //z is up cos ue4 
	} else {
		SrcRot.y = FVector::CrossProduct(dir, FVector::UpVector);  //right
		SrcRot.x = dir;
		SrcRot.z = FVector::CrossProduct(SrcRot.y, SrcRot.x);  //forward
	}
	SrcRot.x.Normalize();
	SrcRot.y.Normalize();
	SrcRot.z.Normalize();

	int maxIter = 9999;
	FVector p0 = normalisePoint(SrcPoints[0].P); //, p1 = normalisePoint(points[1], bnds);
	Points.Add(Point(p0));

	float d = 0;
	for (int i = 1; i < SrcPoints.Num(); i++) {
		FVector p1 = normalisePoint(SrcPoints[i].P);

		float d1 = (p1 - p0).Size();

		//float dt = d1 + d;
		float delta = NormDelta - d;
		while (d1 > delta && maxIter-- > 0) {
			FVector md = p0 + (p1 - p0)*(delta / d1);
			d1 -= delta;
			p0 = md;
			Points.Add(Point(md));
			d = 0;
			delta = NormDelta;
		}
		p0 = p1;
		d += d1;
	}

	for (int i = Points.Num(); i--; ) {
		Point& p1 = Points[i];
		FVector nrm = FVector::ZeroVector;
		if (i > 0)
			nrm += p1.P - Points[i - 1].P;
		if (i < Points.Num() - 1)
			nrm -= p1.P - Points[i + 1].P;

		nrm.Normalize();
		p1.D = nrm;

	}
	NPointCount = Points.Num();
}

float AShape::pDev(const Point &a, const Point &b) const {
	return (a.P - b.P).SizeSquared()*(1.0f - DirFactor) + (a.D - b.D).SizeSquared() * DirFactor;
}

FVector AShape::normalisePoint(const FVector &p)const {
	auto ret = p;
	ret = ret - SrcBnds.GetCenter();
	ret /= SrcBnds.GetSize();
	ret = SrcRot.mulTranspose(ret);
	return ret;
}
FVector AShape::deNormalisePoint(const FVector &p) const {
	auto ret = SrcRot.mul(p);
	ret *= SrcBnds.GetSize();
	ret += SrcBnds.GetCenter();
	return ret;
}

void AShape::drawShape() const {
	if (SrcPoints.Num() < 2) return;

	if (DrawKey == EKeys::Invalid && Recorder->DebugShapeMatch == EDebugShapeMatch::Single && Recorder->SinglePick != this)
		return;

	auto offP = GetActorLocation();
	DrawDebugBox(GetWorld(), SrcBnds.GetCenter() + offP, SrcBnds.GetExtent(), FColor(64, 64, 128));


	auto w = GetWorld();
	if (DrawNormLines) {
		DrawDebugLine(w, SrcBnds.GetCenter() + offP, SrcBnds.GetCenter() + offP + AvgDir *NormScale * 10, FColor(255, 0, 255));
		DrawDebugLine(w, SrcBnds.GetCenter() + offP, SrcBnds.GetCenter() + offP + SrcRot.x *NormScale * 10, FColor(0, 0, 255));
	}
	/*
	DrawDebugLine(w, SrcPoints.Last().P + offP, SrcPoints[0].P + offP, FColor(255, 255, 255));

	auto mid = (SrcPoints.Last().P + SrcPoints[0].P)*0.5f +offP;
	auto vec = (SrcPoints.Last().P - SrcPoints[0].P).UnsafeNormal();
	DrawDebugLine(w, mid, mid + FVector::CrossProduct(vec, FVector::UpVector)*NormScale * 10, FColor(0, 255, 0));
	DrawDebugLine(w, mid, mid + FVector::CrossProduct(vec, FVector::CrossProduct(vec, FVector::UpVector)*NormScale * 10), FColor(0, 255, 255));
	*/
	{
		auto lp = SrcPoints.Last().P + offP;
		for (int i = SrcPoints.Num() - 1; i--; ) {
			auto np = SrcPoints[i].P + offP;
			DrawDebugLine(w, lp, np, FColor(255, 0, 0));
			if (DrawNormLines)
				DrawDebugLine(w, np, np + SrcPoints[i].CtrlrD*NormScale, FColor(255, 0, 255));
			lp = np;
		}

		//	DrawDebugLine(w, lp, SrcPoints[0].P + offP, FColor(0, 0, 255));
	}


	if (Points.Num() < 2) return;

	{
		auto lp = Points.Last();
		for (int i = Points.Num() - 1; i--; ) {
			auto np = Points[i];
			DrawDebugLine(w, deNormalisePoint(lp.P) + offP, deNormalisePoint(np.P) + offP, FColor(0, 255, 0));

			lp = np;
		}
	}

	if (DrawKey == EKeys::Invalid && Recorder->DebugShapeMatch == EDebugShapeMatch::OverlayEach && Recorder->LastEval) 
	{
		drawShadow(*Recorder->LastEval);
	} else if (DrawKey != EKeys::Invalid && Recorder->EvalShadow && Recorder->LastEval) 
	{
		drawShadow(*Recorder->LastEval);
	}



}

void AShape::drawShadow(const AShape & other) const {


	auto wo = GetWorld();
	auto ind = new int[other.Points.Num()];

	for (auto i = other.Points.Num(); i-- > 0;)
		ind[i] = i *Points.Num() / other.Points.Num();

	ind[other.Points.Num() - 1] = Points.Num() - 1;

	FVector lp = FVector::ZeroVector;
	bool flag = false;
	for (int i = other.Points.Num(); i-- > 0; ) {
		float cpd = pDev(other.Points[i], Points[ind[i]]);

		auto sp = deNormalisePoint(other.Points[i].P) + GetActorLocation();
		if (flag) {
			DrawDebugLine(wo, sp, lp, FColor(0, 0, 255));
		} else flag = true;

		DrawDebugLine(wo, sp, deNormalisePoint(Points[ind[i]].P) + GetActorLocation(), FColor(0, 0, 0));
		lp = sp;

	}

	delete[] ind;

}


void AShape::BeginDestroy() {
	Super::BeginDestroy();
	//return;
	if (Recorder->IsValidLowLevel()) {
		if (Recorder->Shapes.Contains(TWeakObjectPtr<AShape>(this)))
			Recorder->Shapes.RemoveSingle(TWeakObjectPtr<AShape>(this));
		if (Recorder->LastEval == this) Recorder->LastEval = nullptr;
		if (Recorder->LastPicked == this) Recorder->LastPicked = nullptr;
	}
}

