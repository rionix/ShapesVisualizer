// Copyright (c) 2003-2022 Rionix, Ltd. All Rights Reserved.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "SimpleVisualizerComponent.generated.h"

//
// ESimpleVisualizerShape - enum of all available shapes
//

UENUM(BlueprintType)
enum class ESimpleVisualizerShape : uint8
{
    Sphere,
    HalfSphere,
    Box,
    Cylinder,
    Cone,
    Capsule,
    // Drawing an array of points with specified radii
    Points,
    // Drawing a polyline by specified array of points
    Polyline
};

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape")
    ESimpleVisualizerShape Shape = ESimpleVisualizerShape::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (ClampMin = "0.0", EditConditionHides, EditCondition = "Shape != ESimpleVisualizerShape::Box && Shape != ESimpleVisualizerShape::Polyline"))
    float Radii = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (EditConditionHides, EditCondition = "Shape == ESimpleVisualizerShape::Cylinder || Shape == ESimpleVisualizerShape::Cone || Shape == ESimpleVisualizerShape::Capsule"))
    float Height = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (EditConditionHides, EditCondition = "Shape == ESimpleVisualizerShape::Box"))
    FVector Extent { 50.f, 50.f, 50.f };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (EditConditionHides, EditCondition = "Shape == ESimpleVisualizerShape::Points || Shape == ESimpleVisualizerShape::Polyline"))
    TArray<FVector> Points;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    FColor Color { 223, 149, 157 };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    bool Wireframe = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", AdvancedDisplay, meta = (ClampMin = "0.0", EditCondition = "Wireframe || Shape == ESimpleVisualizerShape::Polyline"))
    float LineThickness = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", AdvancedDisplay, meta = (ClampMin = "8", ClampMax = "64"))
    int32 NumSides = 24;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool ShowOnlyWhenSelected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool WantsSelectionOutline = true;

public:

    USimpleVisualizerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // UPrimitiveComponent Interface

    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

public:

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetSphereShape(float InRadii = 50.f);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetHalfSphereShape(float InRadii = 50.f);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetBoxShape(const FVector& InExtent);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetCylinderShape(float InRadii = 50.f, float InHeight = 100.f);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetConeShape(float InRadii = 50.f, float InHeight = 100.f);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetCapsuleShape(float InRadii = 50.f, float InHeight = 100.f);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetPointsShape(const TArray<FVector>& InPoints);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetPolylineShape(const TArray<FVector>& InPoints);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetColor(const FColor& InColor = FColor::White);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetWireframe(bool InWireframe, float InLineThickness = 0.f);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleVisualizer")
    void SetNumSides(int32 InNumSides = 24);
};
