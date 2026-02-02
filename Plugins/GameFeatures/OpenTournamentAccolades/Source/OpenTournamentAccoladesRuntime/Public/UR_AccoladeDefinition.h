// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "UR_AccoladeDefinition.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class USoundBase;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FGameAccoladeDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// The message to display
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	// The sound to play
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> Sound;

	// The icon to display
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayThumbnail="true", AllowedClasses="/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses="/Script/Engine.MediaTexture"))
	TSoftObjectPtr<UObject> Icon;

	// Duration (in seconds) to display this accolade
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DisplayDuration = 1.0f;

	// Location to display this accolade
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag LocationTag;

	// Tags associated with this accolade
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AccoladeTags;

	// When this accolade is displayed, any existing displayed/pending accolades with any of
	// these tags will be removed (e.g., getting a triple-elim will suppress a double-elim)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer CancelAccoladesWithTag;
};

/**
 *
 */
UCLASS(BlueprintType)
class UGameAccoladeDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// The sound to play
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound;

	// The icon to display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayThumbnail="true", AllowedClasses="/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses="/Script/Engine.MediaTexture"))
	TObjectPtr<UObject> Icon;

	// Tags associated with this accolade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer AccoladeTags;

	// When this accolade is displayed, any existing displayed/pending accolades with any of
	// these tags will be removed (e.g., getting a triple-elim will suppress a double-elim)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer CancelAccoladesWithTag;
};
