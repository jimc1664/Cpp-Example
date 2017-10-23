// Fill out your copyright notice in the Description page of Project Settings.

#include "Mint.h"
#include "ShapeSave.h"
#include "Shape.h"

void FShapeSaveDat::init(AShape &s)
{
	SrcBnds = s.SrcBnds;
	SrcPoints = s.SrcPoints;
	//SrcBnds.MoveTo(s.GetActorLocation());
	SrcBnds.Min += s.GetActorLocation();
	SrcBnds.Max += s.GetActorLocation();

	Name = s.GetFName();
	Threshold = s.Threshold;
	ShapeType = s.ShapeType;
}

void UShapeSave::init(const TArray<TWeakObjectPtr<AShape>> &shapes) {

	Shapes.SetNum(shapes.Num());

	for (int i = Shapes.Num(); i--;) {
		Shapes[i].init(*shapes[i]);
	}

}






