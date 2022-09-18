// Copyright (c) 2003-2022 Rionix, Ltd. All Rights Reserved.

#include "SimpleVisualizerComponent.h"

//
// FSimpleVisualizerSceneProxy
//

class FSimpleVisualizerSceneProxy : public FPrimitiveSceneProxy
{
public:

    FSimpleVisualizerSceneProxy(const USimpleVisualizerComponent* InComponent)
        : FPrimitiveSceneProxy(InComponent)
        , Extent(InComponent->Extent)
        , Points(InComponent->Points)
        , Color(InComponent->Color)
        , PointRadius(InComponent->PointRadius)
        , Wireframe(InComponent->Wireframe)
        , LineThickness(InComponent->LineThickness)
        , NumSides(InComponent->NumSides)
        , NumRings(InComponent->NumRings)
        , ShowOnlyWhenSelected(InComponent->ShowOnlyWhenSelected)
    {
        bWantsSelectionOutline = InComponent->WantsSelectionOutline;
    }

    virtual SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
    {
        const FVector Radii{ PointRadius };
        const FMatrix& LTW = GetLocalToWorld();
        const FVector WorldOrigin = LTW.GetOrigin();

        const FMaterialRenderProxy* const ParentMaterial = Wireframe ? nullptr
            : GEngine->DebugMeshMaterial->GetRenderProxy();
        const FMaterialRenderProxy* const MeshMaterial = ParentMaterial
            ? new(FMemStack::Get()) FColoredMaterialRenderProxy(ParentMaterial, Color)
            : nullptr;

        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
        {
            FPrimitiveDrawInterface* PDI = Wireframe ? Collector.GetPDI(ViewIndex) : nullptr;

            if (VisibilityMap & (1 << ViewIndex))
            {
                for (const FVector& Pt : Points)
                {
                    if (Wireframe)
                        DrawWireSphere(PDI, WorldOrigin + Pt, Color, PointRadius, NumSides,
                            SDPG_World, LineThickness);
                    else
                        GetSphereMesh(WorldOrigin + Pt, Radii, NumSides, NumRings,
                            MeshMaterial, SDPG_World, false, ViewIndex, Collector);
                }

                if (!Extent.IsNearlyZero())
                {
                    if (Wireframe)
                        DrawOrientedWireBox(PDI, WorldOrigin,
                            LTW.GetScaledAxis(EAxis::X),
                            LTW.GetScaledAxis(EAxis::Y),
                            LTW.GetScaledAxis(EAxis::Z),
                            Extent, Color, SDPG_World, LineThickness);
                    else
                        GetBoxMesh(LTW, Extent, MeshMaterial,
                            SDPG_World, ViewIndex, Collector);
                }
                else if (Points.Num() == 0)
                {
                    if (Wireframe)
                        DrawWireSphere(PDI, WorldOrigin, Color, PointRadius, NumSides,
                            SDPG_World, LineThickness);
                    else
                        GetSphereMesh(WorldOrigin, Radii, NumSides, NumRings, MeshMaterial,
                            SDPG_World, false, ViewIndex, Collector);
                }
            }
        }
    }

    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance Result;
        Result.bDrawRelevance = IsShown(View) && (!ShowOnlyWhenSelected || IsSelected());
        Result.bDynamicRelevance = true;
        Result.bShadowRelevance = IsShadowCast(View);
        Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
        Result.bSeparateTranslucency = Result.bNormalTranslucency = IsShown(View);
        return Result;
    }

    virtual uint32 GetMemoryFootprint(void) const override
    {
        return sizeof(*this) + GetAllocatedSize() + Points.GetAllocatedSize();
    }

protected:

    FVector Extent;
    TArray<FVector> Points;
    FLinearColor Color;
    float PointRadius;
    bool Wireframe;
    float LineThickness;
    int32 NumSides;
    int32 NumRings;
    bool ShowOnlyWhenSelected;
};

//
// USimpleVisualizerComponent
//

USimpleVisualizerComponent::USimpleVisualizerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Tick
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Collision & Navigation
    SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    SetGenerateOverlapEvents(false);
    CanCharacterStepUpOn = ECB_No;
    SetCanEverAffectNavigation(false);

    // Rendering
    SetHiddenInGame(true);
    SetCastShadow(false);
    SetReceivesDecals(false);
}

FPrimitiveSceneProxy* USimpleVisualizerComponent::CreateSceneProxy()
{
    return new FSimpleVisualizerSceneProxy(this);
}

FBoxSphereBounds USimpleVisualizerComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    const bool IsZeroExtent = Extent.IsNearlyZero();
    const FBoxSphereBounds ExtentBounds = IsZeroExtent
        ? FBoxSphereBounds{ FSphere{FVector::ZeroVector, PointRadius} }
        : FBoxSphereBounds{ FBox{ -Extent, Extent } };

    if (Points.Num() > 0)
    {
        const FBoxSphereBounds PointsBounds{ Points.GetData(), static_cast<uint32>(Points.Num()) };
        const FBoxSphereBounds PointsBoundsExp = PointsBounds.ExpandBy(PointRadius);
        return IsZeroExtent
            ? PointsBoundsExp.TransformBy(LocalToWorld)
            : (ExtentBounds + PointsBoundsExp).TransformBy(LocalToWorld);
    }

    return ExtentBounds.TransformBy(LocalToWorld);
}

void USimpleVisualizerComponent::SetExtent(const FVector& NewExtent)
{
    Extent = NewExtent;
    UpdateBounds();
    MarkRenderStateDirty();
}

void USimpleVisualizerComponent::SetPoints(const TArray<FVector>& NewPoints)
{
    Points = NewPoints;
    UpdateBounds();
    MarkRenderStateDirty();
}
