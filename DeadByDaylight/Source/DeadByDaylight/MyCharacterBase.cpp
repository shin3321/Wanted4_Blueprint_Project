// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterBase.h"

// Sets default values
AMyCharacterBase::AMyCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyCharacterBase::OnReceiveMovePacket(FVector NewLocation, FRotator NewRotation)
{
	NewTargetLocation = NewLocation;
	NewTargetRotation = NewTargetRotation;
}

