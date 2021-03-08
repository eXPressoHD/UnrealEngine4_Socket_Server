// Fill out your copyright notice in the Description page of Project Settings.

#include "FugenColor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

bool objectInitialized = false;
UMaterialInstanceDynamic* dynamicTest;

// Sets default values
AFugenColor::AFugenColor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AFugenColor::BeginPlay()
{
	Super::BeginPlay();

	if (!objectInitialized) {
		InitObject();
	}
	
}

void AFugenColor::InitObject() 
 {
	auto Cube = FindComponentByClass<UStaticMeshComponent>();
	auto Material = Cube->GetMaterial(0);

	DynamicMaterial = UMaterialInstanceDynamic::Create(Material, NULL);
	Cube->SetMaterial(0, DynamicMaterial);
	dynamicTest = DynamicMaterial;
	objectInitialized = true;
}

// Called every frame
void AFugenColor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFugenColor::ChangeFugenColor(FLinearColor color)
{
	if (dynamicTest)
	{
		dynamicTest->SetVectorParameterValue(FName("Color"), color); //Neue Farbe des Materials "Color" setzen
	}
}