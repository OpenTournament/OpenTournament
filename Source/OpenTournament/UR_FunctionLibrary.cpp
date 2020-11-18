// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_FunctionLibrary.h"

#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Internationalization/Regex.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/Widget.h"
#include "Components/PanelWidget.h"
#include "Components/MeshComponent.h"

#include "UR_GameModeBase.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_Character.h"
#include "UR_PlayerInput.h"
#include "UR_Weapon.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AGameModeBase* UUR_FunctionLibrary::GetGameModeBaseDefaultObject(const UObject* WorldContextObject)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (const auto GameState = World->GetGameState())
        {
            if (const auto GameModeClass = GameState->GameModeClass)
            {
                return Cast<AGameModeBase>(GameModeClass->GetDefaultObject());
            }
        }
    }
    return nullptr;
}

AUR_GameModeBase* UUR_FunctionLibrary::GetGameModeDefaultObject(const UObject* WorldContextObject)
{
    return GetGameModeDefaultObject<AUR_GameModeBase>(WorldContextObject);
}

FColor UUR_FunctionLibrary::GetPlayerDisplayTextColor(APlayerState* PS)
{
    if (!PS)
    {
        return FColorList::White;
    }
    else if (PS->IsOnlyASpectator())
    {
        return GetSpectatorDisplayTextColor();
    }
    else if (AUR_PlayerState* URPlayerState = Cast<AUR_PlayerState>(PS))
    {
        return URPlayerState->GetColor().ToFColor(true);
    }
    else
    {
        return FColorList::White;	//???
    }
}

FString UUR_FunctionLibrary::StripRichTextDecorators(const FString& InText)
{
    FString Result(TEXT(""), InText.Len());

    int32 CurrentPosition = 0;

    // see RichTextMarkupProcessing.cpp

    TArray<FTextRange> LineRanges;
    FTextRange::CalculateLineRangesFromString(InText, LineRanges);

    const FRegexPattern ElementRegexPattern(TEXT("<([\\w\\d\\.-]+)((?: (?:[\\w\\d\\.-]+=(?>\".*?\")))+)?(?:(?:/>)|(?:>(.*?)</>))"));
    FRegexMatcher ElementRegexMatcher(ElementRegexPattern, InText);

    for (int32 i = 0; i < LineRanges.Num(); i++)
    {
        // Limit the element regex matcher to the current line
        ElementRegexMatcher.SetLimits(LineRanges[i].BeginIndex, LineRanges[i].EndIndex);

        while (ElementRegexMatcher.FindNext())
        {
            // append all from current position up to opening marker
            Result.Append(InText.Mid(CurrentPosition, ElementRegexMatcher.GetMatchBeginning() - CurrentPosition));
            // append inner content
            Result.Append(ElementRegexMatcher.GetCaptureGroup(3));
            // set current position to after closing marker
            CurrentPosition = ElementRegexMatcher.GetMatchEnding();
        }
    }

    Result.Append(InText.Mid(CurrentPosition));

    return Result;
}


bool UUR_FunctionLibrary::IsKeyMappedToAction(const FKey& Key, FName ActionName)
{
    const UUR_PlayerInput* PlayerInput = GetDefault<UUR_PlayerInput>();
    TArray<FInputActionKeyMapping> Mappings;
    if (PlayerInput && PlayerInput->FindUserActionMappings(ActionName, Mappings))
    {
        for (const auto& Mapping : Mappings)
        {
            if (Mapping.Key == Key)
                return true;
        }
    }
    return false;
}

bool UUR_FunctionLibrary::IsKeyMappedToAxis(const FKey& Key, FName AxisName, float Direction)
{
    const UUR_PlayerInput* PlayerInput = GetDefault<UUR_PlayerInput>();
    TArray<FInputAxisKeyMapping> Mappings;
    if (PlayerInput && PlayerInput->FindUserAxisMappings(AxisName, Mappings))
    {
        for (const auto& Mapping : Mappings)
        {
            if (Mapping.Key == Key && (FMath::IsNearlyZero(Direction) || (Direction * Mapping.Scale > 0)))
                return true;
        }
    }
    return false;
}

AUR_PlayerController* UUR_FunctionLibrary::GetLocalPlayerController(const UObject* WorldContextObject)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetFirstPlayerController<AUR_PlayerController>();
    }

    return nullptr;
}

bool UUR_FunctionLibrary::IsLocallyViewed(const AActor* Other)
{
    AUR_PlayerController* PC = GetLocalPlayerController(Other);
    return PC && PC->GetViewTarget() == Other;
}

bool UUR_FunctionLibrary::IsViewingFirstPerson(const AUR_Character* Other)
{
    return Other && IsLocallyViewed(Other) && !Other->bViewingThirdPerson;
}


bool UUR_FunctionLibrary::IsLocallyControlled(const AActor* Other)
{
    return Other && Other->HasLocalNetOwner();
}

bool UUR_FunctionLibrary::IsComponentLocallyControlled(const UActorComponent* Other)
{
    return Other && IsLocallyControlled(Other->GetOwner());
}


FString UUR_FunctionLibrary::GetTimeString(const float TimeSeconds)
{
    // only minutes and seconds are relevant
    const int32 TotalSeconds = FMath::Max(0, FMath::TruncToInt(TimeSeconds) % 3600);
    const int32 NumMinutes = TotalSeconds / 60;
    const int32 NumSeconds = TotalSeconds % 60;

    const FString TimeDesc = FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
    return TimeDesc;
}

UFXSystemComponent* UUR_FunctionLibrary::SpawnEffectAtLocation(const UObject* WorldContextObject, UFXSystemAsset* Template, const FTransform& Transform, bool bAutoDestroy, bool bAutoActivate)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (auto PS = Cast<UParticleSystem>(Template))
        {
            return UGameplayStatics::SpawnEmitterAtLocation(World, PS, Transform, bAutoDestroy, EPSCPoolMethod::None, bAutoActivate);
        }
        else if (auto NS = Cast<UNiagaraSystem>(Template))
        {
            return UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NS, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), bAutoDestroy, bAutoActivate);
        }
    }
    return nullptr;
}

UFXSystemComponent* UUR_FunctionLibrary::SpawnEffectAttached(UFXSystemAsset* Template, const FTransform& Transform, USceneComponent* AttachToComponent, FName AttachPointName, EAttachLocation::Type LocationType, bool bAutoDestroy, bool bAutoActivate)
{
    if (auto PS = Cast<UParticleSystem>(Template))
    {
        return UGameplayStatics::SpawnEmitterAttached(PS, AttachToComponent, AttachPointName, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), LocationType, bAutoDestroy, EPSCPoolMethod::None, bAutoActivate);
    }
    else if (auto NS = Cast<UNiagaraSystem>(Template))
    {
        return UNiagaraFunctionLibrary::SpawnSystemAttached(NS, AttachToComponent, AttachPointName, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), LocationType, bAutoDestroy, ENCPoolMethod::None, bAutoActivate);
    }
    return nullptr;
}


UAnimMontage* UUR_FunctionLibrary::GetCurrentActiveMontageInSlot(UAnimInstance* AnimInstance, FName SlotName, bool& bIsValid, float& Weight)
{
    for (int32 i = AnimInstance->MontageInstances.Num() - 1; i >= 0; i--)
    {
        const FAnimMontageInstance* MontageInstance = AnimInstance->MontageInstances[i];
        if (MontageInstance && MontageInstance->IsActive())
        {
            UAnimMontage* Montage = MontageInstance->Montage;
            if (Montage && Montage->GetAnimationData(SlotName))
            {
                bIsValid = true;
                Weight = MontageInstance->GetWeight();
                return Montage;
            }
        }
    }
    bIsValid = false;
    Weight = 0.f;
    return NULL;
}


void UUR_FunctionLibrary::ParseFloatTextInput(FText Text, bool& bIsNumeric, float& OutValue)
{
    FString Str = Text.ToString().TrimStartAndEnd().Replace(TEXT(" "), TEXT("")).Replace(TEXT(","), TEXT("."));
    bIsNumeric = Str.IsNumeric();
    OutValue = bIsNumeric ? UKismetStringLibrary::Conv_StringToFloat(Str) : 0.f;
}


bool UUR_FunctionLibrary::IsOnlySpectator(APlayerState* PS)
{
    return PS->IsOnlyASpectator();
}

float UUR_FunctionLibrary::GetFloatOption(const FString& Options, const FString& Key, float DefaultValue)
{
    const FString InOpt = UGameplayStatics::ParseOption(Options, Key);
    if (!InOpt.IsEmpty())
    {
        return FCString::Atof(*InOpt);
    }
    return DefaultValue;
}

bool UUR_FunctionLibrary::FindChildrenWidgetsByClass(UWidget* Target, TSubclassOf<UWidget> WidgetClass, TArray<UWidget*>& OutWidgets, bool bRecursive)
{
    if (!WidgetClass)
    {
        return false;
    }

    // Only PanelWidget subclasses can have children
    UPanelWidget* Panel = Cast<UPanelWidget>(Target);
    if (!Panel)
    {
        return false;
    }

    int32 LengthBefore = OutWidgets.Num();

    int32 Count = Panel->GetChildrenCount();
    for (int32 i = 0; i < Count; i++)
    {
        UWidget* Child = Panel->GetChildAt(i);

        if (Child && Child->IsA(WidgetClass))
        {
            OutWidgets.Add(Child);
        }

        if (bRecursive)
        {
            FindChildrenWidgetsByClass(Child, WidgetClass, OutWidgets, true);
        }
    }

    return OutWidgets.Num() > LengthBefore;
}

void UUR_FunctionLibrary::ClearOverrideMaterials(UMeshComponent* MeshComp)
{
    if (MeshComp)
    {
        MeshComp->EmptyOverrideMaterials();
    }
}

void UUR_FunctionLibrary::OverrideAllMaterials(UMeshComponent* MeshComp, UMaterialInterface* Material)
{
    if (MeshComp)
    {
        int32 Num = MeshComp->GetNumMaterials();
        for (int32 i = 0; i < Num; i++)
        {
            MeshComp->SetMaterial(i, Material);
        }
    }
}

void UUR_FunctionLibrary::GetAllWeaponClasses(TSubclassOf<AUR_Weapon> InClassFilter, TArray<TSubclassOf<AUR_Weapon>>& OutWeaponClasses)
{
    // NOTE: this is temporary. We will need proper asset registry management later on.
    for (TObjectIterator<UClass> Itr; Itr; ++Itr)
    {
        UClass* Class = *Itr;
        if ((InClassFilter && Class->IsChildOf(InClassFilter)) || Class->IsChildOf<AUR_Weapon>())
        {
#if WITH_EDITOR
            if (Class->HasAnyFlags(RF_Transient) && Class->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
            {
                continue;
            }
#endif
            OutWeaponClasses.Add(Class);
        }
    }
}
