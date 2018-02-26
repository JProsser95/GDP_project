// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShake.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ToyPlane.generated.h"

class UPossessableActorComponent;

UCLASS()
class GDP_PROJECT_API AToyPlane : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AToyPlane();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	UPossessableActorComponent* possComponent;

	enum CameraType {
		FIRST_PERSON,
		THIRD_PERSON
	};

	// Players initial boost
	UPROPERTY(EditAnywhere, Category = "Plane")
	float MaximumBoost;

	// Players current boost
	UPROPERTY(VisibleAnywhere, Category = "Plane")
	float CurrentBoost;

	// Is the plane currently boosting
	UPROPERTY(EditAnywhere, Category = "Plane")
	bool IsBoosting;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float MinSpeed;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float MaxBoostSpeed;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float CamShakeSpeed;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float SpeedIncrement;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float BoostSpeedIncrement;

	UPROPERTY(EditAnywhere, Category = "Plane")
	bool SwapSwAndArrows;

	UPROPERTY(EditAnywhere, Category = "Plane")
	bool PitchInverted;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float PitchAmount;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float YawAmount;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float RollAmount;

	UPROPERTY(EditAnywhere, Category = "Plane")
	float PropRotateSpeed;

	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FRotator CameraRotation;
	FRotator CameraRotationOffset;

	UPROPERTY(Category = Camera, EditAnywhere)
	bool AutoFocus;

	UPROPERTY(Category = Camera, EditAnywhere)
	float AutoFocusDelay;
	float fLastUnFocusTime;

	// (0-1) Lerp amount per second for changing the rotate speed to the maximum rotation speed
	UPROPERTY(EditAnywhere, Category = "Plane")
	float RotationInterpolation;

	UPROPERTY(EditAnywhere, Category = "Plane")
	bool bIsActive;

	// Resets the camera to the be directly behind the plane
	void UpdateCamera(float DeltaTime);

	float fPropRotation;
	float fSpeed;
	bool bAlreadyRestarted;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when the Pawn is possesed
	virtual void Restart() override;

	// Accessor for MaximumBoost
	UFUNCTION(BlueprintPure, Category = "Plane")
	float GetMaximumBoost() { return MaximumBoost; }

	// Accessor for CurrentBoost
	UFUNCTION(BlueprintPure, Category = "Plane")
	float GetCurrentBoost() { return CurrentBoost; }

	// Accessor for isBoosting
	UFUNCTION(BlueprintPure, Category = "Plane")
	bool GetIsBoosting() { return IsBoosting; }

	/* Updates current boost
	* @param Boost The amount the boost of the plane will change 
	*/
	UFUNCTION(BluePrintCallable, Category = "Plane")
	void UpdateCurrentBoost(float boostIncrement);

	class UCustomMovementComponent* CustomMovementComponent;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void SetIsActive(bool Value);
	bool GetIsActive() { return bIsActive; }

	void RotateDown(float DeltaTime);
	void FlyTowards(FVector targetPosition, float DeltaTime);

	void StartBoost();
protected:

	// The buleprint for the camera shake 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Plane")
	TSubclassOf<UCameraShake> CameraShake;

	// Widget class to use for HUD screen
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Plane", Meta = (BlueprintProtected = true))
	TSubclassOf<class UUSerWidget> HUDWidgetClass;

	// Instance of the HUD
	UPROPERTY()
	class UUserWidget* CurrentWidget;

	UPROPERTY(EditAnywhere)
	USpringArmComponent* OurCameraSpringArm;
	UCameraComponent* OurCamera;

	// Allows the addition of a static mesh component in the editor
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PlaneBodyMeshComponent;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PlanePropMeshComponent;

	CameraType eCameraType;
	
	//Input variables
	FVector4 TargetInput;
	FVector4 MovementInput;
	FVector2D CameraInput;

	//Collision functions
	UFUNCTION()
	void OnToyPlaneOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	void InterpolateMovementInput(float DeltaTime);

	//Input functions
	void Pitch(float AxisValue);
	void Yaw(float AxisValue);
	void Roll(float AxisValue);
	void Throttle(float AxisValue);

	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);

	void EndBoost();
	void CameraZoom();
};
