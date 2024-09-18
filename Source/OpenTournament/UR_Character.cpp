// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Character.h"

#include <TimerManager.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/SkinnedMeshComponent.h>

#include <GameplayTagsManager.h>
#include <InputAction.h>
#include <Components/CapsuleComponent.h>
#include <Engine/DamageEvents.h>
#include <GameFramework/GameState.h>
#include <GameFramework/SpringArmComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include <Net/UnrealNetwork.h>
#include <Perception/AISense_Sight.h>

#include "Camera/CameraComponent.h"

#include "OpenTournament.h"
#include "UR_AbilitySystemComponent.h"
#include "UR_AttributeSet.h"
#include "UR_DamageType.h"
#include "UR_GameMode.h"
#include "UR_GameplayAbility.h"
#include "UR_GameplayTags.h"
#include "UR_InputComponent.h"
#include "UR_InventoryComponent.h"
#include "UR_LogChannels.h"
#include "UR_Logging.h"
#include "UR_PaniniUtils.h"
#include "UR_PawnExtensionComponent.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_Projectile.h"
#include "UR_TeamAgentInterface.h"
#include "UR_UserSettings.h"
#include "UR_Weapon.h"
#include "AbilitySystem/Attributes/UR_HealthSet.h"
#include "AI/AIPerceptionSourceNativeComp.h"
#include "Attributes/UR_CombatSet.h"
#include "Character/UR_CharacterCustomization.h"
#include "Character/UR_CharacterMovementComponent.h"
#include "Character/UR_HealthComponent.h"
#include "Interfaces/UR_ActivatableInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_Character)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Character::AUR_Character(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UUR_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
    , FootstepTimestamp(0.f)
    , FootstepTimeIntervalBase(0.300f)
    , FallDamageScalar(0.15f)
    , FallDamageSpeedThreshold(2675.f)
    , CrouchTransitionSpeed(12.f)
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    // Unreal & UT99 Values (Scaling Factor 2.5)
    GetCapsuleComponent()->InitCapsuleSize(42.5f, 97.5f);

    URMovementComponent = Cast<UUR_CharacterMovementComponent>(GetCharacterMovement());
    URMovementComponent->bUseFlatBaseForFloorChecks = true;

    InventoryComponent = Cast<UUR_InventoryComponent>(CreateDefaultSubobject<UUR_InventoryComponent>(TEXT("InventoryComponent")));

    // FVector(-39.56f, 1.75f, BaseEyeHeight)   <- where does this come from?

    BaseEyeHeight = 64.f;
    DefaultCameraPosition = FVector(-0.f, 0.f, BaseEyeHeight);
    CrouchedEyeHeight = 64.f;

    EyeOffset = FVector(0.f, 0.f, 0.f);
    TargetEyeOffset = EyeOffset;
    EyeOffsetLandingBobMaximum = BaseEyeHeight + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    EyeOffsetLandingBobMinimum = EyeOffsetLandingBobMaximum / 10.f;
    EyeOffsetToTargetInterpolationRate = FVector(18.f, 10.f, 10.f);
    TargetEyeOffsetToNeutralInterpolationRate = FVector(5.f, 5.f, 5.f);

    // Create a zerolength arm for first person camera
    FirstPersonCamArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonCamArm"));
    FirstPersonCamArm->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamArm->SetRelativeLocation(DefaultCameraPosition); // Position the camera
    FirstPersonCamArm->TargetArmLength = 0.f;
    FirstPersonCamArm->bDoCollisionTest = false;
    FirstPersonCamArm->bUsePawnControlRotation = true;

    // Create a CameraComponent
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    //CharacterCameraComponent->SetupAttachment(GetCapsuleComponent());
    //CharacterCameraComponent->SetRelativeLocation(DefaultCameraPosition); // Position the camera
    //CharacterCameraComponent->bUsePawnControlRotation = true;
    FirstPersonCamera->SetupAttachment(FirstPersonCamArm);

    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFirstPerson"));
    MeshFirstPerson->SetOnlyOwnerSee(true);
    MeshFirstPerson->SetupAttachment(FirstPersonCamera);
    MeshFirstPerson->bCastDynamicShadow = false;
    MeshFirstPerson->CastShadow = false;
    MeshFirstPerson->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
    MeshFirstPerson->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

    WeaponAttachPoint = FName(TEXT("GripPoint"));

    // Mesh third person (now using SetVisibility in CameraViewChanged)
    GetMesh3P()->bOwnerNoSee = false;
    GetMesh3P()->bCastHiddenShadow = true;

    // By default, do not refresh animations/bones when not rendered
    GetMesh3P()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
    MeshFirstPerson->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

    // Third person camera
    ThirdPersonArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonArm"));
    ThirdPersonArm->SetupAttachment(GetCapsuleComponent());
    ThirdPersonArm->TargetArmLength = 400.f;
    ThirdPersonArm->TargetOffset.Set(0.f, 0.f, 100.f);
    ThirdPersonArm->bUsePawnControlRotation = true;

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(ThirdPersonArm);

    // Create the attribute set, this replicates by default
    AttributeSet = CreateDefaultSubobject<UUR_AttributeSet>(TEXT("AttributeSet"));
    HealthSet = CreateDefaultSubobject<UUR_HealthSet>(TEXT("HealthSet"));
    CombatSet = CreateDefaultSubobject<UUR_CombatSet>(TEXT("CombatSet"));

    // Create the ASC
    AbilitySystemComponent = CreateDefaultSubobject<UUR_AbilitySystemComponent>("AbilitySystemComponent");

    // AI Perception Source
    AIPerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionSourceNativeComp>("AIPerceptionStimuliSource");
    AIPerceptionStimuliSource->SetAutoRegisterAsSource(true);
    AIPerceptionStimuliSource->SetRegisterAsSourceForSenses({ UAISense_Sight::StaticClass() });

    // Pawn Extension
    PawnExtComponent = CreateDefaultSubobject<UUR_PawnExtensionComponent>(TEXT("PawnExtensionComponent"));
    PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
    PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

    // Health
    HealthComponent = CreateDefaultSubobject<UUR_HealthComponent>(TEXT("HealthComponent"));
    HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
    HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

    // Hair
    HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HairMesh");
    HairMesh->SetupAttachment(GetMesh3P());
    HairMesh->SetLeaderPoseComponent(GetMesh3P());
    HairMesh->bCastHiddenShadow = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, DodgeDirection);
    DOREPLIFETIME(ThisClass, InventoryComponent);
    DOREPLIFETIME(ThisClass, AbilitySystemComponent);
    DOREPLIFETIME(ThisClass, AbilitySystemComponent);
    DOREPLIFETIME(ThisClass, MyTeamID)
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::BeginPlay()
{
    InitializeGameplayTagsManager();

    Super::BeginPlay();

    UUR_PaniniUtils::TogglePaniniProjection(GetMesh1P(), true, true);

    if (GetNetMode() == NM_DedicatedServer)
    {
        // Server considers being never rendered, so it will never update anims/bones when optimization settings are enabled.
        // We have two options here :
        //
        // Option 1 = Always tick pose but never update transforms.
        //            This is most likely better for performance, but means we cannot rely on transforms (location/rotation) of bones (headshot!) nor attached objects (weapons).
        //            The issue can be solved though by calling RefreshBoneTransforms() manually before reading them.
        //
        // Option 2 = Always tick pose and update transforms.
        //            This is easier to work with, but probably less performance friendly.
        //            However here the updating of transforms should benefit from parallelism so hopefully it's not as bad.

        //GetMesh3P()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
        GetMesh3P()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    }
}

void AUR_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickFootsteps(DeltaTime);
    TickEyePosition(DeltaTime);
}

UAbilitySystemComponent* AUR_Character::GetAbilitySystemComponent() const
{
    if (PawnExtComponent == nullptr)
    {
        return nullptr;
    }

    return PawnExtComponent->GetGameAbilitySystemComponent();
}

UUR_AbilitySystemComponent* AUR_Character::GetGameAbilitySystemComponent() const
{
    return Cast<UUR_AbilitySystemComponent>(AbilitySystemComponent);
}

void AUR_Character::OnAbilitySystemInitialized()
{
    UUR_AbilitySystemComponent* ASC = GetGameAbilitySystemComponent();
    check(ASC);

    HealthComponent->InitializeWithAbilitySystem(ASC);

    InitializeGameplayTags();
}

void AUR_Character::OnAbilitySystemUninitialized()
{
    HealthComponent->UninitializeFromAbilitySystem();
}

UInputComponent* AUR_Character::CreatePlayerInputComponent()
{
    static const FName InputComponentName(TEXT("URCharacterInputComponent0"));
    return NewObject<UUR_InputComponent>(this, InputComponentName);
}

void AUR_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PawnExtComponent->SetupPlayerInputComponent();

    //

    if (auto URInputComponent = Cast<UUR_InputComponent>(InputComponent))
    {
        URInputComponent->BindAction(InputActionNextWeapon, ETriggerEvent::Triggered, this, &AUR_Character::NextWeapon);
        URInputComponent->BindAction(InputActionPreviousWeapon, ETriggerEvent::Triggered, this, &AUR_Character::PrevWeapon);
        URInputComponent->BindAction(InputActionDropWeapon, ETriggerEvent::Triggered, this, &AUR_Character::DropWeapon);
    }

    SetupWeaponBindings();

    // Voice
    // Ping
    // Emote
}

void AUR_Character::ApplyCustomization_Implementation(FCharacterCustomization& InCustomization)
{
    UUR_CharacterCustomizationBackend::LoadCharacterCustomizationAssets(InCustomization);

    if (GetMesh3P())
    {
        if (InCustomization.LoadedCharacter)
        {
            GetMesh3P()->SetSkeletalMeshAsset(InCustomization.LoadedCharacter);
            // Important Note :
            // We need to reset materials when swapping meshes, because different meshes might use different material slots...
            // This means the correct materials have to be set in the SkeletalMesh asset directly, NOT in the Character blueprint mesh component!
            // I'm not sure how to best handle this.
            GetMesh3P()->EmptyOverrideMaterials();
        }
        GetMesh3P()->SetVectorParameterValueOnMaterials("Skin tone", FVector(FLinearColor(InCustomization.SkinTone)));
    }

    if (GetMesh1P())
    {
        GetMesh1P()->SetVectorParameterValueOnMaterials("Skin tone", FVector(FLinearColor(InCustomization.SkinTone)));
    }

    if (HairMesh)
    {
        if (InCustomization.LoadedHair)
        {
            HairMesh->SetSkeletalMeshAsset(InCustomization.LoadedHair);
            HairMesh->EmptyOverrideMaterials();
        }
        HairMesh->SetVectorParameterValueOnMaterials("Hair Color", FVector(FLinearColor(InCustomization.HairColor)));
    }

    UpdateTeamColor();  //since we may have reset materials, need to update this
}

void AUR_Character::UpdateTeamColor_Implementation()
{
    if (GetMesh3P())
    {
        GetMesh3P()->SetScalarParameterValueOnMaterials(FName(TEXT("TeamIndex")), IUR_TeamInterface::Execute_GetTeamIndex(this));
        GetMesh3P()->SetScalarParameterValueOnMaterials(FName(TEXT("IsSelf")), Cast<APlayerController>(GetController()) ? 1.f : 0.f);
    }
    if (GetMesh1P())
    {
        GetMesh1P()->SetScalarParameterValueOnMaterials(FName(TEXT("TeamIndex")), IUR_TeamInterface::Execute_GetTeamIndex(this));
        // FirstPerson mesh is always considered "self" and should use AllyColor
        GetMesh1P()->SetScalarParameterValueOnMaterials(FName(TEXT("IsSelf")), 1.f);
    }
}

void AUR_Character::UnPossessed()
{
    AController* const OldController = Controller;

    // Stop listening for changes from the old controller
    const FGenericTeamId OldTeamID = MyTeamID;
    if (IUR_TeamAgentInterface* ControllerAsTeamProvider = Cast<IUR_TeamAgentInterface>(OldController))
    {
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
    }

    Super::UnPossessed();

    // Force stop firing
    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->Deactivate();
    }
    DesiredFireModeNum.Empty();

    PawnExtComponent->HandleControllerChanged();

    // Determine what the new team ID should be afterwards
    //MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
    //ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AUR_Character::OnRep_Controller()
{
    Super::OnRep_Controller();
    PawnExtComponent->HandleControllerChanged();
}

void AUR_Character::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    PawnExtComponent->HandlePlayerStateReplicated();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Camera

void AUR_Character::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    UCameraComponent* Camera = PickCamera();
    if (Camera)
    {
        const bool bThirdPerson = IsThirdPersonCamera(Camera);
        if (bThirdPerson != bViewingThirdPerson)
        {
            bViewingThirdPerson = bThirdPerson;
            CameraViewChanged();
        }
        Camera->GetCameraView(DeltaTime, OutResult);
    }
    else
    {
        // Fallback to Super
        Super::CalcCamera(DeltaTime, OutResult);
    }
}

UCameraComponent* AUR_Character::PickCamera_Implementation()
{
    // End game = always 3p
    AGameState* GS = GetWorld()->GetGameState<AGameState>();
    if (GS && GS->HasMatchEnded() && ThirdPersonCamera)
    {
        return ThirdPersonCamera;
    }

    // Player desires 3p
    if (auto PC = UUR_FunctionLibrary::GetLocalPC<AUR_PlayerController>(this))
    {
        if (PC->bWantsThirdPersonCamera && ThirdPersonCamera && ThirdPersonCamera->IsActive())
            return ThirdPersonCamera;
    }

    // Alive = 1p
    if (IsAlive() && FirstPersonCamera && FirstPersonCamera->IsActive())
    {
        return FirstPersonCamera;
    }

    // Fallback to 3p
    if (ThirdPersonCamera && ThirdPersonCamera->IsActive())
    {
        return ThirdPersonCamera;
    }

    return nullptr;
}

bool AUR_Character::IsThirdPersonCamera_Implementation(UCameraComponent* Camera)
{
    return Camera && Camera == ThirdPersonCamera;
}

void AUR_Character::CameraViewChanged_Implementation()
{
    if (GetMesh3P())
        GetMesh3P()->SetVisibility(bViewingThirdPerson, true);

    if (GetMesh1P())
        GetMesh1P()->SetVisibility(!bViewingThirdPerson, true);

    //NOTE: If visibility propagation works as expected and if the weapon is properly attached to meshes,
    // then it might not be necessary to update weapon visibility. Needs checking out.

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->UpdateMeshVisibility();
    }

    // If a zoom is active, toggle it according to 1P/3P
    if (CurrentZoomInterface)
    {
        IUR_ActivatableInterface::Execute_AIF_SetActive(CurrentZoomInterface.GetObject(), !bViewingThirdPerson, false);
    }
}

void AUR_Character::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
    // Not sure yet how we're gonna handle ThirdPerson weapon firing, but that's gonna be a handful, if we ever want to do it.
    OutLocation = FirstPersonCamera->GetComponentLocation();
    OutRotation = GetViewRotation();
}

void AUR_Character::BecomeViewTarget(APlayerController* PC)
{
    Super::BecomeViewTarget(PC);

    if (PC && PC->IsLocalPlayerController())
    {
        // Update all the things (character 1p/3p mesh, weapon 1p/3p mesh, zooming state)
        bViewingThirdPerson = IsThirdPersonCamera(PickCamera());
        CameraViewChanged();
    }
}

void AUR_Character::EndViewTarget(APlayerController* PC)
{
    Super::EndViewTarget(PC);

    if (PC && PC->IsLocalPlayerController())
    {
        bViewingThirdPerson = true;
        CameraViewChanged();
    }
}

void AUR_Character::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    UpdateMovementPhysicsGameplayTags(PrevMovementMode);

    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

void AUR_Character::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    PawnExtComponent->HandleControllerChanged();

    const FGenericTeamId OldTeamID = MyTeamID;

    // Grab the current team ID and listen for future changes
    if (IUR_TeamAgentInterface* ControllerAsTeamProvider = Cast<IUR_TeamAgentInterface>(NewController))
    {
        MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
    }
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AUR_Character::RegisterZoomInterface(TScriptInterface<IUR_ActivatableInterface> NewZoomInterface)
{
    if (CurrentZoomInterface == NewZoomInterface)
    {
        return;
    }

    // Deactivate previous
    if (CurrentZoomInterface)
    {
        IUR_ActivatableInterface::Execute_AIF_Deactivate(CurrentZoomInterface.GetObject(), false);
    }

    // Set new
    CurrentZoomInterface = NewZoomInterface;

    // Activate if currently viewed in 1P
    if (CurrentZoomInterface && UUR_FunctionLibrary::IsViewingFirstPerson(this))
    {
        IUR_ActivatableInterface::Execute_AIF_Activate(CurrentZoomInterface.GetObject(), false);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Movement

void AUR_Character::MoveForward(const float InValue)
{
    if (InValue != 0.0f)
    {
        const FRotator ControlRotation = GetControlRotation();
        FRotator ForwardRotation = ControlRotation;

        // If we aren't flying / swimming, our ForwardRotation should trim out pitch / roll values
        if (URMovementComponent != nullptr && !URMovementComponent->Is3DMovementMode())
        {
            ForwardRotation = FRotator(0, ControlRotation.Yaw, 0);
        }

        // Move in the direction calculated by our ForwardRotation
        AddMovementInput(FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::X), InValue);
    }
}

void AUR_Character::MoveRight(const float InValue)
{
    if (InValue != 0.0f)
    {
        // Strafing needs no Pitch, so just take our Control Rotation's Yaw
        const FRotator ControlRotation = GetControlRotation();
        const FRotator ForwardRotation(0, ControlRotation.Yaw, 0);

        // Add movement in the perpendicular Y axis
        AddMovementInput(FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::Y), InValue);
    }
}

void AUR_Character::MoveUp(const float InValue)
{
    if (InValue != 0.0f)
    {
        // add movement in up direction
        AddMovementInput(FVector(0.f, 0.f, 1.f), InValue);
    }
}

void AUR_Character::TickFootsteps(const float DeltaTime)
{
    const float VelocityMagnitude = GetCharacterMovement()->Velocity.Size();
    const float WalkingSpeedPercentage = GetCharacterMovement()->MaxWalkSpeed / VelocityMagnitude;
    const float TimeSeconds = GetWorld()->TimeSeconds;
    const float TimeSinceLastFootstep = TimeSeconds - FootstepTimestamp;

    if (GetCharacterMovement()->MovementMode == MOVE_Walking)
    {
        if (VelocityMagnitude > 0.0f && TimeSinceLastFootstep > FootstepTimeIntervalBase * WalkingSpeedPercentage)
        {
            PlayFootstepEffects(WalkingSpeedPercentage);
            FootstepTimestamp = TimeSeconds;
        }
    }
}

void AUR_Character::PlayFootstepEffects(const float WalkingSpeedPercentage) const
{
    const float FootstepVolume = FMath::Clamp<float>(0.2f, 1.f, WalkingSpeedPercentage);
    UGameplayStatics::PlaySound2D(GetWorld(), CharacterVoice.FootstepSound, FootstepVolume, 1.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::SetupWeaponBindings()
{
    if (auto URInputComponent = Cast<UUR_InputComponent>(InputComponent))
    {
        for (auto WeaponBinding : WeaponBindings)
        {
            URInputComponent->BindAction(WeaponBinding.Value, ETriggerEvent::Triggered, this, &AUR_Character::SelectWeapon, WeaponBinding.Key);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::RecalculateBaseEyeHeight()
{
    //NOTE: Careful, this is called every Tick on non-owning clients, not just during crouch events
    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        Super::RecalculateBaseEyeHeight();
        return;
    }

    const float DefaultHalfHeight{ GetDefaultHalfHeight() };
    const float AbsoluteDifference = DefaultHalfHeight - ((GetCharacterMovement()->GetCrouchedHalfHeight() / DefaultHalfHeight) * DefaultHalfHeight);

    if (GetMovementComponent()->IsMovingOnGround())
    {
        CrouchEyeOffsetZ += bIsCrouched ? AbsoluteDifference : -1.f * AbsoluteDifference;
    }
}

void AUR_Character::TickEyePosition(const float DeltaTime)
{
    // Check if Player JustTeleported. If so, ensure the EyeOffset updates immediately
    if (GetCharacterMovement()->bJustTeleported && (FMath::Abs(OldLocationZ - GetActorLocation().Z) > GetCharacterMovement()->MaxStepHeight))
    {
        EyeOffset.Z = 0.f;
    }
    else
    {
        EyeOffset.Z += (OldLocationZ - GetActorLocation().Z);
    }

    // Crouch Stuff
    const float StandingBonus{ bIsCrouched ? CrouchTransitionSpeed : 0.f };
    CrouchEyeOffsetZ = FMath::FInterpTo(CrouchEyeOffsetZ, BaseEyeHeight, DeltaTime, CrouchTransitionSpeed + StandingBonus);

    EyeOffset.X = FMath::FInterpTo(EyeOffset.X, TargetEyeOffset.X, DeltaTime, EyeOffsetToTargetInterpolationRate.X);
    EyeOffset.Y = FMath::FInterpTo(EyeOffset.Y, TargetEyeOffset.Y, DeltaTime, EyeOffsetToTargetInterpolationRate.Y);
    EyeOffset.Z = FMath::FInterpTo(EyeOffset.Z, TargetEyeOffset.Z, DeltaTime, EyeOffsetToTargetInterpolationRate.Z);
    TargetEyeOffset.X = FMath::FInterpTo(TargetEyeOffset.X, 0.f, DeltaTime, TargetEyeOffsetToNeutralInterpolationRate.X);
    TargetEyeOffset.Y = FMath::FInterpTo(TargetEyeOffset.Y, 0.f, DeltaTime, TargetEyeOffsetToNeutralInterpolationRate.Y);
    TargetEyeOffset.Z = FMath::FInterpTo(TargetEyeOffset.Z, 0.f, DeltaTime, TargetEyeOffsetToNeutralInterpolationRate.Z);

    // Force Eye Position when stepping on Movers
    if (auto MovementBase = GetMovementBase())
    {
        if (MovementBase->Mobility == EComponentMobility::Movable)
        {
            EyeOffset.Z = 0.f;
            TargetEyeOffset.Z = 0.f;
        }
    }

    //GAME_LOG(Game, Log, "Ticking EyeOffset: %f, %f, %f)", EyeOffset.X, EyeOffset.Y, EyeOffset.Z);
    FirstPersonCamArm->SetRelativeLocation(FVector(-0.f, 0.f, CrouchEyeOffsetZ) + EyeOffset, false);

    // Update OldLocationZ. Order of operations is important here, this must follow our EyeOffset updates
    OldLocationZ = GetActorLocation().Z;
}

void AUR_Character::LandedViewOffset()
{
    const float FallingVelocityZ = FMath::Abs(URMovementComponent->Velocity.Z);
    if (FallingVelocityZ > (URMovementComponent->JumpZVelocity / 2.f))
    {
        const float LandingBobAlpha = FMath::Clamp<float>(FallingVelocityZ / FallDamageSpeedThreshold, 0.f, 1.f);

        TargetEyeOffset.Z = -1.f * FMath::Lerp(EyeOffsetLandingBobMinimum, EyeOffsetLandingBobMaximum, LandingBobAlpha);
        GAME_LOG(LogGame, Log, "Landing Bob EyeOffset.Z: %f", TargetEyeOffset.Z);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::CheckJumpInput(float DeltaTime)
{
    if (URMovementComponent)
    {
        URMovementComponent->CheckJumpInput(DeltaTime);

        // If we reached this point and the flag indicating a jump is still set, then perform the jump effects
        if (bPressedJump)
        {
            if (JumpCurrentCount == 0)
            {
                JumpCurrentCount++;

                // Play effects for Jump
                OnJumped();
            }
        }
    }
}

void AUR_Character::ClearJumpInput(float DeltaTime)
{
    Super::ClearJumpInput(DeltaTime);
    if (URMovementComponent)
    {
        URMovementComponent->ClearDodgeInput();
    }
}

void AUR_Character::Landed(const FHitResult& Hit)
{
    PlayLandedEffects(Hit);

    TakeFallingDamage(Hit, GetCharacterMovement()->Velocity.Z);

    GAME_LOG(LogGame, Log, "Updating OldLocationZ: %f (Old), %f (New)", OldLocationZ, GetActorLocation().Z);
    OldLocationZ = GetActorLocation().Z;

    FootstepTimestamp = GetWorld()->TimeSeconds;

    // Landing View Bob
    LandedViewOffset();

    Super::Landed(Hit);
}

void AUR_Character::PlayLandedEffects(const FHitResult& Hit)
{
    if (LandedParticleSystemClass)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LandedParticleSystemClass, Hit.Location, Hit.Normal.ToOrientationRotator());
    }

    const float LandingVolume = FMath::Clamp<float>(0.2f, 1.f, FMath::Abs(GetVelocity().Z) / FallDamageSpeedThreshold);
    UGameplayStatics::PlaySound2D(GetWorld(), CharacterVoice.FootstepSound, LandingVolume, 1.f);
}

void AUR_Character::TakeFallingDamage(const FHitResult& Hit, float FallingSpeed)
{
    if (AbilitySystemComponent == nullptr || URMovementComponent == nullptr)
    {
        return;
    }

    if (FallingSpeed * -1.f > FallDamageSpeedThreshold)
    {
        const float FallingDamage = (-1.f * FallDamageScalar) * (FallDamageSpeedThreshold + FallingSpeed);
        GAME_LOG(LogGame, Log, "Character received Fall Damage (%f)!", FallingDamage);

        if (FallingDamage >= 1.0f)
        {
            FDamageEvent DamageEvent; // @! TODO Real DamageTypes
            TakeDamage(FallingDamage, DamageEvent, Controller, this);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    OnStartCrouchEffects();

    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    CrouchEyeOffset.Z += 64.f - BaseEyeHeight + HalfHeightAdjust; // @! TODO 64.f = StartingBaseEyeHeight

    OldLocationZ = GetActorLocation().Z;

    UpdateGameplayTags(FGameplayTagContainer{ }, FGameplayTagContainer{ GetMovementActionGameplayTag(EMovementAction::Crouching) });

    // Anims, sounds
}

void AUR_Character::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    OnEndCrouchEffects();

    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    CrouchEyeOffset.Z += 64.f - BaseEyeHeight + HalfHeightAdjust; // @! TODO 64.f = StartingBaseEyeHeight

    OldLocationZ = GetActorLocation().Z;

    UpdateGameplayTags(FGameplayTagContainer{ GetMovementActionGameplayTag(EMovementAction::Crouching) }, FGameplayTagContainer{ });

    // Anims, sounds
}

void AUR_Character::OnStartCrouchEffects()
{
    UGameplayStatics::PlaySound2D(GetWorld(), CrouchTransitionSound, 1.0f, 1.f);
}

void AUR_Character::OnEndCrouchEffects()
{
    UGameplayStatics::PlaySound2D(GetWorld(), CrouchTransitionSound, 1.0f, 1.f);
}

void AUR_Character::ToggleCrouch()
{
    const UUR_CharacterMovementComponent* MovementComponent = CastChecked<UUR_CharacterMovementComponent>(GetCharacterMovement());

    if (bIsCrouched || MovementComponent->bWantsToCrouch)
    {
        UnCrouch();
    }
    else if (MovementComponent->IsMovingOnGround())
    {
        Crouch();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_Character::IsDodgeCapable_Implementation() const
{
    return (URMovementComponent != nullptr);
}

bool AUR_Character::IsDodgePermitted_Implementation() const
{
    bool IsPermitted = false;

    if (URMovementComponent != nullptr)
    {
        IsPermitted = !URMovementComponent->IsFlying();
        IsPermitted = IsPermitted && !URMovementComponent->bIsDodging;
        IsPermitted = IsPermitted && GetWorld()->TimeSeconds > URMovementComponent->DodgeResetTime;
    }

    return IsPermitted;
}

bool AUR_Character::CanDodge() const
{
    return IsDodgeCapable() && IsDodgePermitted();
}

void AUR_Character::Dodge(FVector DodgeDir, FVector DodgeCross)
{
    if (CanDodge())
    {
        if (DodgeOverride(DodgeDir, DodgeCross))
        {
            return;
        }

        URMovementComponent->PerformDodge(DodgeDir, DodgeCross);
        return;
    }

    if (URMovementComponent)
    {
        URMovementComponent->ClearDodgeInput();
    }
}

void AUR_Character::DodgeTest(const EDodgeDirection InDodgeDirection)
{
    // @! TODO Testing only...

    SetDodgeDirection(InDodgeDirection);
}

void AUR_Character::OnDodge_Implementation(const FVector& DodgeLocation, const FVector& DodgeDir)
{
    // @! TODO Effects
    if (CharacterVoice.DodgeSound != nullptr)
    {
        if (GetLocalRole() == ROLE_Authority)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.DodgeSound, GetActorLocation(), GetActorRotation());
        }
    }
}

void AUR_Character::OnWallDodge_Implementation(const FVector& DodgeLocation, const FVector& DodgeDir)
{
    // @! TODO Effects
    if (CharacterVoice.DodgeSound != nullptr)
    {
        if (GetLocalRole() == ROLE_Authority)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.DodgeSound, GetActorLocation(), GetActorRotation());

            // Modify player view for Dodge
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float AUR_Character::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // @! TODO: Deprecating most of this logic in favor of GameplayEffect modification...

    const float OriginalDamage = Damage;

    GAME_LOG(LogGame, Log, "Damage Incoming (%f)", OriginalDamage);

    //NOTE:
    // Super() takes care of :
    // - low level checks (returns 0.0 if not pass)
    // - calculate proper splash damage values (original Damage is the full damage regardless of radius)
    // - impart physics impulses to physics components
    //
    //NOTE: Low level checks include an authority check, meaning Super() only imparts components physics on server side,
    // which makes no sense. We might have to rework this.
    //
    //NOTE: Either way, impulses are only for physics-enabled components.
    // There is nothing builtin for character knockback, so we will have to do it ourselves.
    // We can access the projectile or the weapon (hitscan) trough DamageCauser, and fetch knockback properties there.
    // Calculate falloff manually, then apply knockback manually using CharacterMovement->AddImpulse.

    // Calculate damage with falloff if this is splash
    Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    // Floor
    Damage = FMath::FloorToFloat(Damage);

    if (Damage == 0.f)
    {
        return 0.f;
    }

    GAME_LOG(LogGame, Log, "Damage pre-hook = %f", Damage);

    const float Falloff = Damage / OriginalDamage;

    // Calculate knockback power
    float KnockbackPower = 1500.f * OriginalDamage; //TODO: this is where we should fetch projectile/weapon's knockback value
    KnockbackPower *= Falloff;

    // Gamemode hook
    if (AUR_GameMode* URGameMode = GetWorld()->GetAuthGameMode<AUR_GameMode>())
    {
        URGameMode->ModifyDamage(Damage, KnockbackPower, this, EventInstigator, DamageEvent, DamageCauser);

        if (Damage == 0.f)
        {
            return 0.f;
        }
    }

    GAME_LOG(LogGame, Log, "Damage post-hook = %f", Damage);

    ////////////////////////////////////////////////////////////
    // Apply Damage

    float DamageToShield = 0.f;
    float DamageToArmor = 0.f;
    float DamageToHealth = 0.f;
    float DamageRemaining = Damage;

    // @! TODO : This block should probably not be handled in our Character but instead via DamageExecution or GameplayEffect
    // if (HealthComponent && !HealthComponent->IsDeadOrDying())
    // {
    //     if (AttributeSet)
    //     {
    //         const float CurrentShield = FMath::FloorToFloat(AttributeSet->Shield.GetCurrentValue());
    //         if (CurrentShield > 0.f)
    //         {
    //             DamageToShield = FMath::Min(DamageRemaining, CurrentShield);
    //             AttributeSet->SetShield(CurrentShield - DamageToShield);
    //             DamageRemaining -= DamageToShield;
    //         }
    //
    //         const float CurrentArmor = FMath::FloorToFloat(AttributeSet->Armor.GetCurrentValue());
    //         const float ArmorAbsorption = AttributeSet->ArmorAbsorptionPercent.GetCurrentValue();
    //         if (CurrentArmor > 0.f && DamageRemaining > 0.f)
    //         {
    //             DamageToArmor = FMath::Min(DamageRemaining * ArmorAbsorption, CurrentArmor);
    //             DamageToArmor = FMath::FloorToFloat(DamageToArmor);
    //             AttributeSet->SetArmor(CurrentArmor - DamageToArmor);
    //             DamageRemaining -= DamageToArmor;
    //         }
    //     }
    //
    //     const float CurrentHealth = HealthComponent->GetHealth();
    //     if (CurrentHealth > 0.f && DamageRemaining > 0.f)
    //     {
    //         DamageToHealth = FMath::Min(DamageRemaining, CurrentHealth);
    //         // AttributeSet->SetHealth(CurrentHealth - DamageToHealth); // @! TODO HealthComponentFix
    //         DamageRemaining -= DamageToHealth;
    //     }
    // }

    GAME_LOG(LogGame, Log, "Damage repartition: Shield(%f), Armor(%f), Health(%f), Extra(%f)", DamageToShield, DamageToArmor, DamageToHealth, DamageRemaining);

    ////////////////////////////////////////////////////////////
    // Knockback & replicated info

    FReplicatedDamageEvent RepDamageEvent;
    RepDamageEvent.Type = DamageEvent.GetTypeID();
    RepDamageEvent.Damage = Damage;
    RepDamageEvent.HealthDamage = DamageToHealth;
    RepDamageEvent.ArmorDamage = DamageToShield + DamageToArmor;
    RepDamageEvent.DamageInstigator = EventInstigator ? EventInstigator->GetPawn() : nullptr;
    if (UKismetMathLibrary::ClassIsChildOf(DamageEvent.DamageTypeClass, UUR_DamageType::StaticClass()))
    {
        RepDamageEvent.DamType = GetDefault<UUR_DamageType>(DamageEvent.DamageTypeClass);
    }
    else
    {
        // Avoid null damagetype, it is annoying to handle in blueprints
        RepDamageEvent.DamType = GetDefault<UUR_DamageType>();
    }

    // NOTE: Adding impulses to CharacterMovement are just velocity changes.
    // Point or Radial doesn't make any difference.
    // Those only matter for skeletal physics (ragdoll), which we may apply on client.
    FVector KnockbackDir;

    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)&DamageEvent;

        KnockbackDir = PointDamageEvent->ShotDirection;

        RepDamageEvent.Location = PointDamageEvent->HitInfo.ImpactPoint;
        RepDamageEvent.Knockback = KnockbackPower * PointDamageEvent->ShotDirection;
    }
    else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {
        FRadialDamageEvent* RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;

        KnockbackDir = (GetActorLocation() - RadialDamageEvent->Origin).GetSafeNormal();

        RepDamageEvent.Location = RadialDamageEvent->Origin;
        RepDamageEvent.Knockback = FVector(KnockbackPower / Falloff, RadialDamageEvent->Params.GetMaxRadius(), 0);
    }
    else
    {
        // This is most likely a code-generated FDamageEvent() with no real source.
        KnockbackDir = FVector::ZeroVector;
        RepDamageEvent.Location = GetActorLocation();
        RepDamageEvent.Knockback = FVector::ZeroVector;
    }

    // When character is on ground, bias knockback towards +Z to lift him
    if (GetCharacterMovement()->IsMovingOnGround() && KnockbackDir.Z < 0.1f)
    {
        KnockbackDir = FVector(KnockbackDir.X, KnockbackDir.Y, FMath::Max(0.1, KnockbackDir.Z + 0.33));
        KnockbackDir.Normalize();
    }

    FVector FinalKnockback = KnockbackPower * KnockbackDir;
    if (FinalKnockback.Size() > 100.f)
    {
        GetCharacterMovement()->AddImpulse(FinalKnockback);
    }

    ////////////////////////////////////////////////////////////
    // Event

    // @! TODO Limit Pain by time, vary by damage etc.
    /*
    if (DamageToHealth > 10.f)
    {
        //TODO: Replicate
        UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.PainSound, GetActorLocation(), GetActorRotation());
    }
    */

    MulticastDamageEvent(RepDamageEvent);

    ////////////////////////////////////////////////////////////
    // Death

    if (HealthComponent && !HealthComponent->IsDeadOrDying())
    {
        // Can use DamageRemaining here to GIB
        Die(EventInstigator, DamageEvent, DamageCauser, RepDamageEvent);
    }

    return Damage;
}

void AUR_Character::MulticastDamageEvent_Implementation(const FReplicatedDamageEvent RepDamageEvent)
{
    OnDamageReceived.Broadcast(this, RepDamageEvent);

    //NOTE: Using pawn as instigator is not ideal because the owner of a projectile can die during projectile flight,
    // and then the player would not be notified about the hit.
    // Controller is not good either because spectators cannot access spectated player's controller.
    // PlayerState might be the way to go.

    if (AUR_Character* Dealer = Cast<AUR_Character>(RepDamageEvent.DamageInstigator))
    {
        Dealer->OnDamageDealt.Broadcast(this, RepDamageEvent);
    }
}

void AUR_Character::Die(AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser, const FReplicatedDamageEvent& RepDamageEvent)
{
    // Already killed (might happen when multiple damage sources in 1 frame)
    if (GetTearOff() || IsPendingKillPending())
    {
        return;
    }

    AUR_GameMode* URGameMode = GetWorld()->GetAuthGameMode<AUR_GameMode>();
    if (URGameMode)
    {
        AController* Killed = GetController();

        // @! TODO : Commenting out this block, as this approach should probably be handled in another way via GAS GameplayEffect
        //if (URGameMode->PreventDeath(Killed, Killer, DamageEvent, DamageCauser))
        //{
            // Make sure we don't stay with <=0 health or IsAlive() would return false.
            //if (AttributeSet->Health_D.GetCurrentValue() <= 0)
            //{
                //AttributeSet->SetHealth(1);
            //}
            //return;
        //}

        URGameMode->PlayerKilled(Killed, Killer, DamageEvent, DamageCauser);
    }

    // Clear inventory
    InventoryComponent->OwnerDied();
    DesiredFireModeNum.Empty();

    // Stop being a target for AIs
    AIPerceptionStimuliSource->UnregisterFromPerceptionSystem();

    // Replicate
    MulticastDied(Killer, RepDamageEvent);

    // Cut the replication link
    TearOff();

    if (IsNetMode(NM_DedicatedServer))
    {
        //Destroy();
        // On dedicated server, we could theoretically destroy right away, but need to give it a bit of slack to replicate PlayDeath & TearOff.
        SetLifeSpan(0.200f);
        SetActorEnableCollision(false);

        // Event on server
        OnDeath.Broadcast(this, Killer);
    }
}

void AUR_Character::PlayDeath_Implementation(AController* Killer, const FReplicatedDamageEvent& RepDamageEvent)
{
    if (IsNetMode(NM_DedicatedServer))
        return;

    APlayerController* OldController = nullptr;
    if (Controller && Controller->GetPawn() == this)
        OldController = Cast<APlayerController>(Controller);

    // Unpossess
    DetachFromControllerPendingDestroy();

    // Set view target back, should auto to 3p cam
    if (OldController)
        OldController->SetViewTarget(this);

    // Improve camera feel
    ThirdPersonArm->bInheritPitch = false;
    ThirdPersonArm->bInheritRoll = false;
    ThirdPersonArm->bEnableCameraLag = true;
    ThirdPersonArm->bEnableCameraRotationLag = true;

    GetCharacterMovement()->DisableMovement();

    // Disable capsule
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));

    // Set mesh as root with physics (ragdoll)
    GetMesh()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
    GetMesh()->SetSimulatePhysics(true);
    SetRootComponent(GetMesh());

    // Attach capsule back to mesh
    GetCapsuleComponent()->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));

    // Apply knockback to ragdoll
    if (RepDamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        GetMesh()->AddImpulseAtLocation(RepDamageEvent.Knockback, RepDamageEvent.Location);
    }
    else if (RepDamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {
        GetMesh()->AddRadialImpulse(RepDamageEvent.Location, RepDamageEvent.Knockback.Y, RepDamageEvent.Knockback.X, ERadialImpulseFalloff::RIF_Linear, false);
    }

    // NOTE: we can only set lifespan after receiving TornOff(), otherwise it is locked by Authority.
    // I am not sure if we can guarantee the order of the two calls here, so handle both.
    bPlayingDeath = true;
    SetLifeSpan(5.0f);

    // Event on clients
    OnDeath.Broadcast(this, Killer);
}

void AUR_Character::OnDeathStarted(AActor*)
{
    DisableMovementAndCollision();
}

void AUR_Character::OnDeathFinished(AActor*)
{
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}


void AUR_Character::DisableMovementAndCollision()
{
    if (Controller)
    {
        Controller->SetIgnoreMoveInput(true);
    }

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    check(CapsuleComp);
    CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

    UUR_CharacterMovementComponent* GameMovementComponent = CastChecked<UUR_CharacterMovementComponent>(GetCharacterMovement());
    GameMovementComponent->StopMovementImmediately();
    GameMovementComponent->DisableMovement();
}

void AUR_Character::DestroyDueToDeath()
{
    K2_OnDeathFinished();

    UninitAndDestroy();
}


void AUR_Character::UninitAndDestroy()
{
    if (GetLocalRole() == ROLE_Authority)
    {
        DetachFromControllerPendingDestroy();
        SetLifeSpan(0.1f);
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if (UUR_AbilitySystemComponent* GameASC = GetGameAbilitySystemComponent())
    {
        if (GameASC->GetAvatarActor() == this)
        {
            PawnExtComponent->UninitializeAbilitySystem();
        }
    }

    SetActorHiddenInGame(true);
}

void AUR_Character::TornOff()
{
    Super::TornOff();

    SetLifeSpan(bPlayingDeath ? 5.0f : 0.200f);
}

bool AUR_Character::IsAlive() const
{
    if (GetTearOff() || IsPendingKillPending() || AttributeSet == nullptr)
    {
        return false;	// server link has been cut, health might not replicate anymore
    }

    if (IsValid(HealthComponent))
    {
        return !HealthComponent->IsDeadOrDying();
    }
    return false;
}

void AUR_Character::ServerSuicide_Implementation()
{
    Die(nullptr, FDamageEvent(), nullptr, FReplicatedDamageEvent());
}


/////////////////////////////////////////////////////////////////////////////////////////////////

FName AUR_Character::GetWeaponAttachPoint() const
{
    return WeaponAttachPoint;
}

USkeletalMeshComponent* AUR_Character::GetPawnMesh() const
{
    return MeshFirstPerson;
}

UUR_InventoryComponent* AUR_Character::GetInventoryComponent()
{
    return InventoryComponent;
}

void AUR_Character::PawnStartFire(uint8 FireModeNum)
{
    /*
    bIsFiring = true;

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->LocalStartFire();
    }
    */

    DesiredFireModeNum.Insert(FireModeNum, 0);

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->RequestStartFire(FireModeNum);
    }
}

void AUR_Character::PawnStopFire(uint8 FireModeNum)
{
    /*
    bIsFiring = false;

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->LocalStopFire();
    }
    */

    DesiredFireModeNum.RemoveSingle(FireModeNum);

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->RequestStopFire(FireModeNum);

        if (DesiredFireModeNum.Num() > 0)
        {
            InventoryComponent->ActiveWeapon->RequestStartFire(DesiredFireModeNum[0]);
        }
    }
}

void AUR_Character::SelectWeapon(int32 Index)
{
    if (InventoryComponent)
    {
        InventoryComponent->SelectWeapon(Index);
    }
}

void AUR_Character::NextWeapon()
{
    if (InventoryComponent)
    {
        InventoryComponent->NextWeapon();
    }
}

void AUR_Character::PrevWeapon()
{
    if (InventoryComponent)
    {
        InventoryComponent->PrevWeapon();
    }
}

void AUR_Character::DropWeapon()
{
    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ServerDropActiveWeapon();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_Character::Server_GiveAbility_Validate(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel)
{
    return true;
}

void AUR_Character::Server_GiveAbility_Implementation(TSubclassOf<UUR_GameplayAbility> InAbility, const int32 InAbilityLevel)
{
    if (InAbility == nullptr)
    {
        GAME_LOG(LogGame, Error, "InAbilityClass was Null!");
        return;
    }

    check(AbilitySystemComponent);

    if (AbilitySystemComponent)
    {
        // @! TODO : Unsure of the purpose of InputID, seems to be related to input/keybinding
        const int32 InputID{ 0 }; //static_cast<int32>(Cast<UUR_GameplayAbility>(InAbilityClass.GetDefaultObject())->Input);

        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(InAbility, InAbilityLevel, InputID, this));
        GameplayAbilities.AddUnique(InAbility);
    }
}

bool AUR_Character::Server_RemoveAbility_Validate(TSubclassOf<UUR_GameplayAbility> InAbilityClass)
{
    return true;
}

void AUR_Character::Server_RemoveAbility_Implementation(TSubclassOf<UUR_GameplayAbility> InAbilityClass) const
{
    if (InAbilityClass == nullptr)
    {
        GAME_LOG(LogGame, Error, "InAbilityClass was Null!");
        return;
    }

    check(AbilitySystemComponent);

    if (AbilitySystemComponent)
    {
        const auto& Abilities = AbilitySystemComponent->GetActivatableAbilities();

        for (const auto& Ability : Abilities)
        {
            if (Ability.Ability->GetClass() == InAbilityClass)
            {
                AbilitySystemComponent->ClearAbility(Ability.Handle);
                break;
            }
        }
    }
}

int32 AUR_Character::GetAbilityLevel(TSubclassOf<UUR_GameplayAbility> InAbilityClass) const
{
    int32 OutLevel{ -1 };

    if (AbilitySystemComponent)
    {
        const auto& Abilities = AbilitySystemComponent->GetActivatableAbilities();

        for (const auto& Ability : Abilities)
        {
            if (Ability.Ability->GetClass() == InAbilityClass)
            {
                OutLevel = Ability.Level;
                break;
            }
        }
    }

    return OutLevel;
}

bool AUR_Character::Server_SetAbilityLevel_Validate(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel)
{
    return true;
}

void AUR_Character::Server_SetAbilityLevel_Implementation(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel)
{
    if (AbilitySystemComponent)
    {
        // Get a reference to our ASC ActiveatableAbilities. Reference so our modifications stick.
        auto& Abilities = AbilitySystemComponent->GetActivatableAbilities();

        for (auto& Ability : Abilities)
        {
            if (Ability.Ability->GetClass() == InAbilityClass)
            {
                Ability.Level = InAbilityLevel;
                AbilitySystemComponent->MarkAbilitySpecDirty(Ability);
                break;
            }
        }
    }
}

void AUR_Character::InitializeMovementActionGameplayTags()
{
    MovementActionGameplayTags.Add(EMovementAction::Jumping, URGameplayTags::TAG_Character_States_Movement_Jumping);
    MovementActionGameplayTags.Add(EMovementAction::Dodging, URGameplayTags::TAG_Character_States_Movement_Dodging);
    MovementActionGameplayTags.Add(EMovementAction::Crouching, URGameplayTags::TAG_Character_States_Movement_Crouching);
    MovementActionGameplayTags.Add(EMovementAction::Running, URGameplayTags::TAG_Character_States_Movement_Running);
}

void AUR_Character::InitializeMovementModeGameplayTags()
{
    MovementModeGameplayTags.Add(EMovementMode::MOVE_None, FGameplayTag{ });
    MovementModeGameplayTags.Add(EMovementMode::MOVE_Custom, FGameplayTag{ });
    MovementModeGameplayTags.Add(EMovementMode::MOVE_MAX, FGameplayTag{ });
    MovementModeGameplayTags.Add(EMovementMode::MOVE_Walking, URGameplayTags::TAG_Character_States_Physics_Walking);
    MovementModeGameplayTags.Add(EMovementMode::MOVE_NavWalking, URGameplayTags::TAG_Character_States_Physics_Walking);
    MovementModeGameplayTags.Add(EMovementMode::MOVE_Falling, URGameplayTags::TAG_Character_States_Physics_Falling);
    MovementModeGameplayTags.Add(EMovementMode::MOVE_Swimming, URGameplayTags::TAG_Character_States_Physics_Swimming);
    MovementModeGameplayTags.Add(EMovementMode::MOVE_Flying, URGameplayTags::TAG_Character_States_Physics_Flying);
}

void AUR_Character::InitializeGameplayTags()
{
    InitializeMovementActionGameplayTags();

    InitializeMovementModeGameplayTags();
}

void AUR_Character::InitializeGameplayTagsManager()
{
    if (!GameplayTagsManager)
    {
        GameplayTagsManager = &UGameplayTagsManager::Get();

        InitializeGameplayTags();
    }
}

FGameplayTag AUR_Character::GetMovementActionGameplayTag(const EMovementAction InMovementAction)
{
    return *MovementActionGameplayTags.Find(InMovementAction);
}

FGameplayTag AUR_Character::GetMovementModeGameplayTag(const EMovementMode InMovementMode)
{
    if (const UWorld* World{ GetWorld() })
    {
        if (const AGameStateBase* GameState{ World->GetGameState() })
        {
            if (GameState->HasMatchStarted())
            {
                return *MovementModeGameplayTags.Find(InMovementMode);
            }
        }
    }

    return FGameplayTag{ };
}

void AUR_Character::UpdateMovementPhysicsGameplayTags(const EMovementMode PreviousMovementMode)
{
    const TEnumAsByte<EMovementMode> MovementMode = GetCharacterMovement()->MovementMode;
    const FGameplayTagContainer TagsToAdd{ GetMovementModeGameplayTag(MovementMode) };
    const FGameplayTagContainer TagsToRemove{ GetMovementModeGameplayTag(PreviousMovementMode) };

    UpdateGameplayTags(TagsToRemove, TagsToAdd);
}

void AUR_Character::UpdateGameplayTags(const FGameplayTagContainer& TagsToRemove, const FGameplayTagContainer& TagsToAdd)
{
#if WITH_EDITOR
    FString TagsToPrint{ };
    for (const FGameplayTag& Tag : GameplayTags)
    {
        TagsToPrint.Append(Tag.ToString() + ", ");
    }
    GAME_LOG(LogGame, Verbose, "Pre: Character (%s) has Tags: %s", *this->GetName(), *TagsToPrint);
#endif

    GameplayTags.RemoveTags(TagsToRemove);
    GameplayTags.AppendTags(TagsToAdd);

#if WITH_EDITOR
    TagsToPrint.Empty();
    for (const FGameplayTag& Tag : GameplayTags)
    {
        TagsToPrint.Append(Tag.ToString() + ", ");
    }
    GAME_LOG(LogGame, Verbose, "Post: Character (%s) has Tags: %s", *this->GetName(), *TagsToPrint);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 AUR_Character::GetTeamIndex_Implementation()
{
    if (const auto PS = GetPlayerState<AUR_PlayerState>())
    {
        return IUR_TeamInterface::Execute_GetTeamIndex(PS);
    }
    return -1;
}

void AUR_Character::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    if (auto PS = GetPlayerState<AUR_PlayerState>())
    {
        IUR_TeamInterface::Execute_SetTeamIndex(PS, NewTeamIndex);
    }
}

void AUR_Character::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    const FGenericTeamId MyOldTeamID = MyTeamID;
    MyTeamID = IntegerToGenericTeamId(NewTeam);
    ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void AUR_Character::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
