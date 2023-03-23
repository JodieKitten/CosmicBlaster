#pragma once

UENUM(BlueprintType)
enum class EFlagType : uint8
{
    EFT_RedFlag UMETA(DisplayName = "Red Flag"),
    EFT_BlueFlag UMETA(DisplayName = "Blue Flag"),
    EFT_MAX UMETA(DisplayName = "Default MAX"),
};