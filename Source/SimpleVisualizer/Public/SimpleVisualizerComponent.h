// Copyright (c) 2003-2022 Rionix, Ltd. All Rights Reserved.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "SimpleVisualizerComponent.generated.h"

//
// USimpleShapeComponent
//

UCLASS(Blueprintable, ClassGroup=Utility,
    hideCategories = (Activation, Lighting, Navigation, Physics, Collision, Tags, Cooking),
    meta=(BlueprintSpawnableComponent))
class USimpleVisualizerComponent : public UPrimitiveComponent
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    FVector Extent = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    TArray<FVector> Points;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor Color { .74f, .3f, .34f, 1.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool Wireframe = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", AdvancedDisplay, meta = (ClampMin = "1.0"))
    float PointRadius = 32.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", AdvancedDisplay, meta = (EditCondition = "Wireframe", ClampMin = "0.0"))
    float LineThickness = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", AdvancedDisplay, meta = (ClampMin = "3", ClampMax = "128"))
    int32 NumSides = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", AdvancedDisplay, meta = (ClampMin = "3", ClampMax = "128"))
    int32 NumRings = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool ShowOnlyWhenSelected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool WantsSelectionOutline = false;

public:

    USimpleVisualizerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // UPrimitiveComponent Interface

#if !UE_BUILD_SHIPPING
    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
#endif
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

    // USimpleShapeComponent

    UFUNCTION(BlueprintCallable, Category = "Components|Simple Shape")
    void SetExtent(const FVector& NewExtent);

    UFUNCTION(BlueprintCallable, Category = "Components|Simple Shape")
    void SetPoints(const TArray<FVector>& NewPoints);
};
