// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Announcement.generated.h"

class UTextBlock;
class UCanvas;

UCLASS()
class COSMICBLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupTime;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* ControllerGuide;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* KeyboardGuide;
};
