// Fill out your copyright notice in the Description page of Project Settings.

#include "ToyPlane.h"
#include "Blueprint/UserWidget.h"
#include "GDP_ProjectGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Macros.h"
#include "CustomMovementComponent.h"

// Sets default values
AToyPlane::AToyPlane()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//	PlaneBodyMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AToyPlane::OnToyPlaneOverlap);
	//	PlaneBodyMeshComponent->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics, true);

	//Create our components

	//Mesh
	PlaneBodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneBodyMeshComponent"));
	PlaneBodyMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	PlaneBodyMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AToyPlane::OnToyPlaneOverlap);
	PlaneBodyMeshComponent->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics, true);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAssetBody(TEXT("StaticMesh'/Game/Plane/PlaneHull.PlaneHull'"));
	if (MeshAssetBody.Object)
		PlaneBodyMeshComponent->SetStaticMesh(MeshAssetBody.Object);

	RootComponent = PlaneBodyMeshComponent;

	//Mesh
	PlanePropMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanePropMeshComponent"));
	PlanePropMeshComponent->AttachToComponent(PlaneBodyMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAssetBProp(TEXT("StaticMesh'/Game/Plane/PlaneProp.PlaneProp'"));
	if (MeshAssetBProp.Object)
		PlanePropMeshComponent->SetStaticMesh(MeshAssetBProp.Object);
	PlanePropMeshComponent->SetRelativeLocation(FVector(-1.0f, 0.0f, 5.0f));

	PlaneBodyMeshComponent->SetRelativeRotation(FRotator(0,90.0f,0));

	OurCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	OurCameraSpringArm->SetupAttachment(RootComponent);
	OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-30.0f, 0.0f, 0.0f));
	OurCameraSpringArm->TargetArmLength = 700.f;
	OurCameraSpringArm->bEnableCameraLag = true;
	OurCameraSpringArm->CameraLagSpeed = 3.0f;

	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	OurCamera->SetupAttachment(OurCameraSpringArm, USpringArmComponent::SocketName);

	PlaneWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	PlaneWidget->SetupAttachment(RootComponent);
	static ConstructorHelpers::FClassFinder<UUserWidget> Widget(TEXT("/Game/HUD/VehicleWidget"));
	PlaneWidget->SetWidgetClass(Widget.Class);
	PlaneWidget->SetRelativeLocation(FVector(0, 0, 150.0f));

	eCameraType = THIRD_PERSON;

	fInitialBoost = 100.0f;
	fCurrentBoost = fInitialBoost;
	fSpeed = 400.0f;
	bIsBoosting = false;
	bIsActive = false;
	fPropRotation = 0.0f;

	//Take control of the default Player
	//AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create an instance of our movement component, and tell it to update our root component.
	CustomMovementComponent = CreateDefaultSubobject<UCustomMovementComponent>(TEXT("CustomMovementComponent"));
	CustomMovementComponent->UpdatedComponent = RootComponent;
}

void AToyPlane::Restart()
{
	Super::Restart();

	AGDP_ProjectGameModeBase* GameMode = (AGDP_ProjectGameModeBase*)GetWorld()->GetAuthGameMode();
	GameMode->ChangeHUD("ToyPlane");
}

// Called when the game starts or when spawned
void AToyPlane::BeginPlay()
{
	Super::BeginPlay();

	
}

// Called every frame
void AToyPlane::Tick(float DeltaTime)
{
	if (!bIsActive)
	{
		FVector PlayerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), PlayerLoc);
		PlaneWidget->SetRelativeRotation(PlayerRot);
		return;
	}

	Super::Tick(DeltaTime);

	

	if (fCurrentBoost <= 0)
		bIsBoosting = false;
	else if (fCurrentBoost > fInitialBoost)
		fCurrentBoost = fInitialBoost;


	if (bIsBoosting) {
		fSpeed += DeltaTime * 500.0f;
		UpdateCurrentBoost(-DeltaTime * 0.3f * fInitialBoost);
	}
	else {
		fSpeed -= DeltaTime * 500.0f;
		UpdateCurrentBoost(DeltaTime * 0.2f * fInitialBoost);
	}

	if (fSpeed > 800.0f) {
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		PC->ClientPlayCameraShake(CameraShake, 1);
	}

	fSpeed = FMath::Clamp(fSpeed, 400.0f, 1000.0f);

	//Scale our movement input axis values by 100 units per second
	MovementInput = MovementInput.GetSafeNormal() * 100.0f;
	FRotator NewRotation = GetActorRotation();

	if (MovementInput.Y != 0) {
		//A or D pressed
		NewRotation.Roll += MovementInput.Y * DeltaTime;
		NewRotation.Roll = FMath::Clamp(NewRotation.Roll, -90.0f, 90.0f);
		NewRotation.Yaw += MovementInput.Y * DeltaTime;
	}
	else {
		NewRotation = FMath::Lerp(NewRotation, FRotator(0, NewRotation.Yaw, 0), 2.0f * DeltaTime);
	}

	if (MovementInput.X != 0) {
		//W or S press
		NewRotation.Pitch += MovementInput.X * DeltaTime;
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, -60.0f, 60.0f);
	}
	else {
		NewRotation = FMath::Lerp(NewRotation, FRotator(0, NewRotation.Yaw, 0), 2.0f * DeltaTime);
	}

	SetActorRotation(NewRotation);
	//SetActorLocation(NewLocation);

	CustomMovementComponent->AddInputVector(GetActorForwardVector() * fSpeed);

	fPropRotation += DeltaTime * 500.0f;
	PlanePropMeshComponent->SetRelativeRotation(FRotator(0, 0, fPropRotation));
}

UPawnMovementComponent* AToyPlane::GetMovementComponent() const
{
	return CustomMovementComponent;
}

void AToyPlane::OnToyPlaneOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
}

void AToyPlane::Posses()
{
	GetWorld()->GetFirstPlayerController()->Possess(this);
}

// Called to bind functionality to input
void AToyPlane::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("PlaneBoost", IE_Pressed, this, &AToyPlane::StartBoost);
	PlayerInputComponent->BindAction("PlaneBoost", IE_Released, this, &AToyPlane::EndBoost);
	PlayerInputComponent->BindAction("PlaneCameraZoom", IE_Released, this, &AToyPlane::CameraZoom);

	//Hook up every-frame handling for our four axes
	PlayerInputComponent->BindAxis("PlaneMoveUp", this, &AToyPlane::MoveUp);
	PlayerInputComponent->BindAxis("PlaneMoveRight", this, &AToyPlane::MoveRight);
	PlayerInputComponent->BindAxis("PlaneCameraPitch", this, &AToyPlane::PitchCamera);
	PlayerInputComponent->BindAxis("PlaneCameraYaw", this, &AToyPlane::YawCamera);
}

//Input functions
void AToyPlane::MoveUp(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AToyPlane::MoveRight(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AToyPlane::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void AToyPlane::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}

void AToyPlane::StartBoost() 
{
	if (fCurrentBoost >= 10.0f) {
		bIsBoosting = true;
	}
}

void AToyPlane::EndBoost()
{
	bIsBoosting = false;
}

void AToyPlane::CameraZoom()
{
	if (eCameraType == FIRST_PERSON) {
		eCameraType = THIRD_PERSON;
		OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-30.0f, 0.0f, 0.0f));
		OurCameraSpringArm->TargetArmLength = 700.f;
		OurCameraSpringArm->bEnableCameraLag = true;
	}
	else {
		eCameraType = FIRST_PERSON;
		OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
		OurCameraSpringArm->TargetArmLength = 0.0f;
		OurCameraSpringArm->bEnableCameraLag = false;
	}
}

void AToyPlane::UpdateCurrentBoost(float currentBoost)
{
	fCurrentBoost += currentBoost;
}

	
