// Copyright (c) 2003-2022 rionix. All Rights Reserved.

#include "Components/ShapesVisualizerComponent.h"
#include "Engine/Engine.h"
#include "Engine/CollisionProfile.h"
#include "Materials/Material.h"
#include "PrimitiveSceneProxy.h"
#include "Runtime/Launch/Resources/Version.h"

//
// Internal functions
//

namespace
{
    // Engine source 4.27
    // .\Engine\Source\Runtime\Engine\Private\PrimitiveDrawingUtils.cpp
    // Lines [1134-1152]: DrawWireChoppedCone
    void DrawWireCone_Internal(FPrimitiveDrawInterface* PDI, const FVector& Base,
        const FVector& X, const FVector& Y, const FVector& Z,
        const FLinearColor& Color, float Radius, float TopRadius, float HalfHeight,
        int32 NumSides, uint8 DepthPriority, float Thickness)
    {
        const float AngleDelta = 2.0f * PI / NumSides;
        FVector LastVertex = Base + X * Radius;
        FVector LastTopVertex = Base + X * TopRadius;

        for (int32 SideIndex = 0; SideIndex < NumSides; SideIndex++)
        {
            const FVector Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
            const FVector TopVertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * TopRadius;

            PDI->DrawLine(LastVertex - Z * HalfHeight, Vertex - Z * HalfHeight, Color, DepthPriority, Thickness);
            PDI->DrawLine(LastTopVertex + Z * HalfHeight, TopVertex + Z * HalfHeight, Color, DepthPriority, Thickness);
            PDI->DrawLine(LastVertex - Z * HalfHeight, LastTopVertex + Z * HalfHeight, Color, DepthPriority, Thickness);

            LastVertex = Vertex;
            LastTopVertex = TopVertex;
        }
    }

    // Engine source 4.27
    // .\Engine\Source\Runtime\Engine\Private\PrimitiveDrawingUtils.cpp
    // Lines [549-650]: BuildCylinderVerts
    void BuildConeVerts_Internal(const FVector& Base,
        const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis,
        float Radius, float HalfHeight, uint32 Sides,
        TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices)
    {
        const float AngleDelta = 2.0f * PI / Sides;
        FVector LastVertex = Base + XAxis * Radius;

        FVector2D TC = FVector2D(0.0f, 0.0f);
        float TCStep = 1.0f / Sides;

        FVector TopOffset = HalfHeight * ZAxis;

        int32 BaseVertIndex = OutVerts.Num();

        //Compute vertices for base circle.
        for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
        {
            const FVector Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) + YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
            FVector Normal = Vertex - Base;
            Normal.Normalize();

            FDynamicMeshVertex MeshVertex;

#if ENGINE_MAJOR_VERSION == 5
            MeshVertex.Position = FVector3f(Vertex - TopOffset);
            MeshVertex.TextureCoordinate[0] = FVector2f(TC);

            MeshVertex.SetTangents(
                (FVector3f)-ZAxis,
                FVector3f((-ZAxis) ^ Normal),
                (FVector3f)Normal
            );
#else
            MeshVertex.Position = Vertex - TopOffset;
            MeshVertex.TextureCoordinate[0] = TC;

            MeshVertex.SetTangents(
                -ZAxis,
                (-ZAxis) ^ Normal,
                Normal
            );
#endif

            OutVerts.Add(MeshVertex); //Add bottom vertex

            LastVertex = Vertex;
            TC.X += TCStep;
        }

        LastVertex = Base + XAxis * Radius;
        TC = FVector2D(0.0f, 1.0f);

        //Compute vertices for the top circle
        for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
        {
            const FVector Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) + YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) * SMALL_NUMBER;
            FVector Normal = Vertex - Base;
            Normal.Normalize();

            FDynamicMeshVertex MeshVertex;

#if ENGINE_MAJOR_VERSION == 5
            MeshVertex.Position = FVector3f(Vertex + TopOffset);
            MeshVertex.TextureCoordinate[0] = FVector2f(TC);

            MeshVertex.SetTangents(
                (FVector3f)-ZAxis,
                FVector3f((-ZAxis) ^ Normal),
                (FVector3f)Normal
            );
#else
            MeshVertex.Position = Vertex + TopOffset;
            MeshVertex.TextureCoordinate[0] = TC;

            MeshVertex.SetTangents(
                -ZAxis,
                (-ZAxis) ^ Normal,
                Normal
            );
#endif

            OutVerts.Add(MeshVertex); //Add top vertex

            LastVertex = Vertex;
            TC.X += TCStep;
        }

        //Add top/bottom triangles, in the style of a fan.
        //Note if we wanted nice rendering of the caps then we need to duplicate the vertices and modify
        //texture/tangent coordinates.
        for (uint32 SideIndex = 1; SideIndex < Sides; SideIndex++)
        {
            int32 V0 = BaseVertIndex;
            int32 V1 = BaseVertIndex + SideIndex;
            int32 V2 = BaseVertIndex + ((SideIndex + 1) % Sides);

            //bottom
            OutIndices.Add(V0);
            OutIndices.Add(V1);
            OutIndices.Add(V2);

            // top
            // OutIndices.Add(Sides + V2);
            // OutIndices.Add(Sides + V1);
            // OutIndices.Add(Sides + V0);
        }

        //Add sides.

        for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
        {
            int32 V0 = BaseVertIndex + SideIndex;
            int32 V1 = BaseVertIndex + ((SideIndex + 1) % Sides);
            int32 V2 = V0 + Sides;
            int32 V3 = V1 + Sides;

            OutIndices.Add(V0);
            OutIndices.Add(V2);
            OutIndices.Add(V1);

            OutIndices.Add(V2);
            OutIndices.Add(V3);
            OutIndices.Add(V1);
        }

    }

    // Engine source 4.27
    // .\Engine\Source\Runtime\Engine\Private\PrimitiveDrawingUtils.cpp
    // Lines [688-697]: GetConeMesh
    void GetConeMesh_Internal(const FMatrix& LocalToWorld,
        float Radius, float HalfHeight, uint32 Sides,
        const FMaterialRenderProxy* MaterialRenderProxy, uint8 DepthPriority,
        int32 ViewIndex, FMeshElementCollector& Collector)
    {
        TArray<FDynamicMeshVertex> MeshVerts;
        TArray<uint32> MeshIndices;
        BuildConeVerts_Internal(FVector::ZeroVector, FVector::XAxisVector, FVector::YAxisVector, FVector::ZAxisVector, 
            Radius, HalfHeight, Sides, MeshVerts, MeshIndices);
        FDynamicMeshBuilder MeshBuilder(Collector.GetFeatureLevel());
        MeshBuilder.AddVertices(MeshVerts);
        MeshBuilder.AddTriangles(MeshIndices);
        MeshBuilder.GetMesh(LocalToWorld, MaterialRenderProxy, DepthPriority, false, false, ViewIndex, Collector);
    }

    // Engine source 4.27
    // .\Engine\Source\Runtime\Engine\Private\PrimitiveDrawingUtils.cpp
    // Lines [699-710]: GetCapsuleMesh
    void GetCapsuleMesh_Internal(const FMatrix& LocalToWorld, const FLinearColor& Color, float Radius, float HalfHeight, int32 NumSides, const FMaterialRenderProxy* MaterialRenderProxy, uint8 DepthPriority, bool bDisableBackfaceCulling, int32 ViewIndex, FMeshElementCollector& Collector)
    {
        const FVector XAxis = LocalToWorld.GetScaledAxis(EAxis::X);
        const FVector YAxis = LocalToWorld.GetScaledAxis(EAxis::Y);
        const FVector ZAxis = LocalToWorld.GetScaledAxis(EAxis::Z);
        const FVector Origin = LocalToWorld.GetOrigin() - ZAxis * HalfHeight;
        const FVector ScaledRadius = LocalToWorld.GetScaleVector() * Radius;
        const float HalfAxis = FMath::Max<float>(HalfHeight - Radius, 1.f);
        const FVector BottomEnd = Origin + Radius * ZAxis;
        const FVector TopEnd = BottomEnd + (2 * HalfAxis) * ZAxis;
        const float CylinderHalfHeight = (TopEnd - BottomEnd).Size() * 0.5;
        const FVector CylinderLocation = BottomEnd + CylinderHalfHeight * ZAxis;

        GetOrientedHalfSphereMesh(TopEnd, FRotationMatrix::MakeFromXY(XAxis, YAxis).Rotator(), ScaledRadius, NumSides, NumSides, 0, PI / 2, MaterialRenderProxy, DepthPriority, bDisableBackfaceCulling, ViewIndex, Collector);
        GetCylinderMesh(CylinderLocation, XAxis, YAxis, ZAxis, Radius, CylinderHalfHeight, NumSides, MaterialRenderProxy, DepthPriority, ViewIndex, Collector);
        GetOrientedHalfSphereMesh(BottomEnd, FRotationMatrix::MakeFromXY(XAxis, YAxis).Rotator(), ScaledRadius, NumSides, NumSides, PI / 2, PI, MaterialRenderProxy, DepthPriority, bDisableBackfaceCulling, ViewIndex, Collector);
    }
}

//
// FShapesVisualizerSceneProxy
//

class FShapesVisualizerSceneProxy : public FPrimitiveSceneProxy
{
public:

    FShapesVisualizerSceneProxy(const UShapesVisualizerComponent* InComponent)
        : FPrimitiveSceneProxy(InComponent)
        , Shape(InComponent->Shape)
        , Radii(InComponent->Radii)
        , Height(InComponent->Height)
        , Extent(InComponent->Extent)
        , Points(InComponent->Points)
        , BaseColor(InComponent->Color)
        , Wireframe(SafeWireframe(InComponent->Shape, InComponent->Wireframe))
        , LineThickness(InComponent->LineThickness)
        , NumSides(InComponent->NumSides)
        , ShowOnlyWhenSelected(InComponent->ShowOnlyWhenSelected)
    {
        bWantsSelectionOutline = InComponent->WantsSelectionOutline;
    }

    virtual SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
        const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
        FMeshElementCollector& Collector) const override
    {
        const FMatrix& LTW = GetLocalToWorld();
        const FVector WorldOrigin = LTW.GetOrigin();
        const float HalfHeight = Height / 2.f;

        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
        {
            if (!(VisibilityMap & (1 << ViewIndex)))
                continue;

            FPrimitiveDrawInterface* PDI = Wireframe ? Collector.GetPDI(ViewIndex) : nullptr;

            const bool Outline = Wireframe && WantsSelectionOutline();
            const FLinearColor Color = GetViewSelectionColor(BaseColor, *Views[ViewIndex],
                Outline ? IsSelected() : false, Outline ? IsHovered() : false,
                false, IsIndividuallySelected());
            const FMaterialRenderProxy* const ParentMaterial = Wireframe ? nullptr
                : GEngine->DebugMeshMaterial->GetRenderProxy();
            const FMaterialRenderProxy* const MeshMaterial = ParentMaterial
                ? new(FMemStack::Get()) FColoredMaterialRenderProxy(ParentMaterial, Color)
                : nullptr;

            switch (Shape)
            {
            case EVisualShape::Sphere:
                if (Wireframe)
                    DrawWireSphere(PDI, FTransform{ LTW },
                        Color, Radii, NumSides,
                        SDPG_World, LineThickness);
                else
                    GetOrientedHalfSphereMesh(WorldOrigin,
                        LTW.Rotator(), LTW.GetScaleVector() * Radii,
                        NumSides, FMath::Max(3, NumSides / 2), 0.f, PI,
                        MeshMaterial, SDPG_World, false, ViewIndex, Collector);
                break;

            case EVisualShape::HalfSphere:
                GetOrientedHalfSphereMesh(WorldOrigin,
                    LTW.Rotator(), LTW.GetScaleVector() * Radii,
                    NumSides, NumSides, 0.f, HALF_PI,
                    MeshMaterial, SDPG_World, false, ViewIndex, Collector);
                break;

            case EVisualShape::Box:
                if (Wireframe)
                    DrawOrientedWireBox(PDI, WorldOrigin,
                        LTW.GetScaledAxis(EAxis::X),
                        LTW.GetScaledAxis(EAxis::Y),
                        LTW.GetScaledAxis(EAxis::Z),
                        Extent, Color, SDPG_World, LineThickness);
                else
                    GetBoxMesh(LTW, Extent,
                        MeshMaterial, SDPG_World, ViewIndex, Collector);
                break;

            case EVisualShape::Cylinder:
                if (Wireframe)
                {
                    if (Height > 0.f)
                        DrawWireCylinder(PDI, WorldOrigin,
                            LTW.GetScaledAxis(EAxis::X),
                            LTW.GetScaledAxis(EAxis::Y),
                            LTW.GetScaledAxis(EAxis::Z),
                            Color, Radii, HalfHeight, NumSides,
                            SDPG_World, LineThickness);
                    else
                        DrawCircle(PDI, WorldOrigin,
                            LTW.GetScaledAxis(EAxis::X),
                            LTW.GetScaledAxis(EAxis::Y),
                            Color, Radii, NumSides,
                            SDPG_World, LineThickness);
                }
                else
                    GetCylinderMesh(LTW, FVector::ZeroVector,
                        FVector::XAxisVector, FVector::YAxisVector, FVector::ZAxisVector,
                        Radii, HalfHeight, NumSides,
                        MeshMaterial, SDPG_World, ViewIndex, Collector);
                break;

            case EVisualShape::Cone:
                if (Wireframe)
                    DrawWireCone_Internal(PDI, WorldOrigin,
                        LTW.GetScaledAxis(EAxis::X),
                        LTW.GetScaledAxis(EAxis::Y),
                        LTW.GetScaledAxis(EAxis::Z),
                        Color, Radii, 0.f, HalfHeight, NumSides,
                        SDPG_World, LineThickness);
                else
                    GetConeMesh_Internal(LTW,
                        Radii, HalfHeight, NumSides,
                        MeshMaterial, SDPG_World, ViewIndex, Collector);
                break;

            case EVisualShape::Capsule:
                if (Wireframe)
                    DrawWireCapsule(PDI, WorldOrigin,
                        LTW.GetScaledAxis(EAxis::X),
                        LTW.GetScaledAxis(EAxis::Y),
                        LTW.GetScaledAxis(EAxis::Z),
                        Color, Radii, HalfHeight, NumSides,
                        SDPG_World, LineThickness);
                else
                    GetCapsuleMesh_Internal(LTW,
                        FLinearColor{ Color }, Radii, HalfHeight, NumSides,
                        MeshMaterial, SDPG_World, false, ViewIndex, Collector);
                break;

            case EVisualShape::Points:
                for (const FVector& Pt : Points)
                {
                    if (Wireframe)
                        DrawWireDiamond(PDI,
                            FTranslationMatrix{ LTW.TransformPosition(Pt) }, Radii,
                            Color, SDPG_World, LineThickness);
                    else
                        GetSphereMesh(LTW.TransformPosition(Pt), FVector{ Radii },
                            NumSides, NumSides,
                            MeshMaterial, SDPG_World, false, ViewIndex, Collector);
                }
                break;

            case EVisualShape::Polyline:
                for (int32 i = 0; i < Points.Num() - 1; ++i)
                {
                    PDI->DrawLine(
                        LTW.TransformPosition(Points[i]),
                        LTW.TransformPosition(Points[i + 1]),
                        Color, SDPG_World, LineThickness);
                }
                break;
            } // switch (Shape)
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

private:

    FORCEINLINE static bool SafeWireframe(EVisualShape Shape, bool Wireframe)
    {
        switch (Shape)
        {
        case EVisualShape::HalfSphere:
            return false;
        case EVisualShape::Polyline:
            return true;
        }
        return Wireframe;
    }

private:

    // Shape
    EVisualShape Shape;
    float Radii;
    float Height;
    FVector Extent;
    TArray<FVector> Points;
    // Appearance
    FColor BaseColor;
    bool Wireframe;
    float LineThickness;
    int32 NumSides;
    bool ShowOnlyWhenSelected;
};

//
// UShapesVisualizerComponent
//

UShapesVisualizerComponent::UShapesVisualizerComponent(const FObjectInitializer& ObjectInitializer)
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

FPrimitiveSceneProxy* UShapesVisualizerComponent::CreateSceneProxy()
{
    return new FShapesVisualizerSceneProxy(this);
}

FBoxSphereBounds UShapesVisualizerComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    switch (Shape)
    {
    case EVisualShape::Sphere:
        return FBoxSphereBounds{ LocalToWorld.GetLocation(), FVector{ Radii }, Radii };
    case EVisualShape::HalfSphere:
        return FBoxSphereBounds{ FVector{ 0.f, 0.f, Radii / 2.f },
            FVector{ Radii, Radii, Radii / 2.f }, Radii }.TransformBy(LocalToWorld);
    case EVisualShape::Box:
        return FBoxSphereBounds{ FBox{ -Extent, Extent } }.TransformBy(LocalToWorld);
    case EVisualShape::Cylinder:
    case EVisualShape::Cone:
    case EVisualShape::Capsule:
    {
        const float HalfHeight = Height / 2.f;
        const FVector BoxExtent{ Radii, Radii, HalfHeight };
        return FBoxSphereBounds(FVector::ZeroVector, BoxExtent, HalfHeight).TransformBy(LocalToWorld);
    }
    case EVisualShape::Points:
    case EVisualShape::Polyline:
    {
        const FBoxSphereBounds PointsBounds{ Points.GetData(), static_cast<uint32>(Points.Num()) };
        return PointsBounds.ExpandBy(Radii).TransformBy(LocalToWorld);
    }
    }
    return FBoxSphereBounds{ LocalToWorld.GetLocation(), FVector::ZeroVector, 0.f };
}

//
// Setters
//

void UShapesVisualizerComponent::SetSphereShape(float InRadii)
{
    Shape = EVisualShape::Sphere;
    Radii = InRadii;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetHalfSphereShape(float InRadii)
{
    Shape = EVisualShape::HalfSphere;
    Radii = InRadii;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetBoxShape(const FVector& InExtent)
{
    Shape = EVisualShape::Box;
    Extent = InExtent;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetCylinderShape(float InRadii, float InHeight)
{
    Shape = EVisualShape::Cylinder;
    Radii = InRadii;
    Height = InHeight;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetConeShape(float InRadii, float InHeight)
{
    Shape = EVisualShape::Cone;
    Radii = InRadii;
    Height = InHeight;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetCapsuleShape(float InRadii, float InHeight)
{
    Shape = EVisualShape::Capsule;
    Radii = InRadii;
    Height = InHeight;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetPointsShape(const TArray<FVector>& InPoints)
{
    Shape = EVisualShape::Points;
    Points = InPoints;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetPolylineShape(const TArray<FVector>& InPoints)
{
    Shape = EVisualShape::Polyline;
    Points = InPoints;
    UpdateBounds();
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetColor(const FColor& InColor)
{
    Color = InColor;
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetWireframe(bool InWireframe, float InLineThickness)
{
    Wireframe = InWireframe;
    LineThickness = FMath::Max(0.f, InLineThickness);
    MarkRenderStateDirty();
}

void UShapesVisualizerComponent::SetNumSides(int32 InNumSides)
{
    NumSides = FMath::Clamp(InNumSides, 8, 64);
    MarkRenderStateDirty();
}
