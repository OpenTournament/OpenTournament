// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CommonLocalPlayer.h>

#include "UR_TeamAgentInterface.h"

#include "UR_LocalPlayer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UInputMappingContext;
struct FSwapAudioOutputResult;
class UUR_SettingsLocal;
class UUR_SettingsShared;
class UUR_MessageHistory;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_LocalPlayer : public UCommonLocalPlayer, public IUR_TeamAgentInterface
{
    GENERATED_BODY()

    UUR_LocalPlayer();

public:
    //~UObject interface
    virtual void PostInitProperties() override;
    //~End of UObject interface

    //~UPlayer interface
    virtual void SwitchController(class APlayerController* PC) override;
    //~End of UPlayer interface

    //~ULocalPlayer interface
    virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;

    virtual void InitOnlineSession() override;
    //~End of ULocalPlayer interface


    //~IUR_TeamAgentInterface interface
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

    virtual FGenericTeamId GetGenericTeamId() const override;

    virtual FOnGameTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
    //~End of IUR_TeamAgentInterface interface

    /** Gets the local settings for this player, this is read from config files at process startup and is always valid */
    UFUNCTION()
    UUR_SettingsLocal* GetLocalSettings() const;

    /** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
    UFUNCTION()
    UUR_SettingsShared* GetSharedSettings() const;

    /** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
    void LoadSharedSettingsFromDisk(bool bForceLoad = false);

protected:
    void OnSharedSettingsLoaded(UUR_SettingsShared* LoadedOrCreatedSettings);

    void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);

    UFUNCTION()
    void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

    void OnPlayerControllerChanged(APlayerController* NewController);

    UFUNCTION()
    void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);


    UPROPERTY(Transient)
    mutable TObjectPtr<UUR_SettingsShared> SharedSettings;

    FUniqueNetIdRepl NetIdForSharedSettings;

    UPROPERTY(Transient)
    mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

    UPROPERTY()
    FOnGameTeamIndexChangedDelegate OnTeamChangedDelegate;

    UPROPERTY()
    TWeakObjectPtr<APlayerController> LastBoundPC;

    ////

public:
    /**
    * Reference to message history subobject.
    */
    UPROPERTY(BlueprintReadOnly)
    UUR_MessageHistory* MessageHistory;

    UPROPERTY(Config, BlueprintReadOnly)
    FString PlayerName;

    virtual FString GetNickname() const override;

    UFUNCTION(BlueprintCallable)
    void SavePlayerName(const FString& NewName)
    {
        PlayerName = NewName;
        SaveConfig();
    }
};
