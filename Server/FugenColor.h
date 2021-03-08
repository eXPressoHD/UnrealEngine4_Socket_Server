// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FugenColor.generated.h"

class UMaterialInstanceDynamic;

UCLASS()
class BASICCODINGUE_API AFugenColor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFugenColor();	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void ChangeFugenColor(FLinearColor color);

private:
	UMaterialInstanceDynamic* DynamicMaterial;

private:
	void InitObject();

};
