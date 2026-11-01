// HADI: VICE CITY
#include "UI/HadiMinimap.h"

#include "Core/HadiCity.h"
#include "Core/HadiMinimap.h"
#include "World/HadiCoreBridge.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Font.h"
#include "RenderUtils.h"

void UHadiMinimap::Initialize(UObject* WorldContext, const hadi::CityModel* InModel, UFont* InFont, int32 Width, int32 Height)
{
	Model = InModel;
	Font = InFont;
	Target = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(WorldContext, UCanvasRenderTarget2D::StaticClass(), Width, Height);
	if (Target)
	{
		Target->ClearColor = HadiBridge::ToLinear(hadi::mappal::Water);
		Target->OnCanvasRenderTargetUpdate.AddDynamic(this, &UHadiMinimap::OnDraw);
	}
	// Draw order: terrain first, buildings last.
	DrawOrder.Reset();
	if (Model)
	{
		for (int32 Layer = 0; Layer < static_cast<int32>(hadi::MapLayer::Count); ++Layer)
		{
			for (size_t Index = 0; Index < Model->map.size(); ++Index)
			{
				if (static_cast<int32>(Model->map[Index].layer) == Layer)
				{
					DrawOrder.Add(static_cast<int32>(Index));
				}
			}
		}
	}
}

UTexture* UHadiMinimap::GetTexture() const
{
	return Target;
}

void UHadiMinimap::Render(const FHadiMinimapState& InState)
{
	State = InState;
	if (Target)
	{
		Target->UpdateResource();
	}
}

void UHadiMinimap::OnDraw(UCanvas* Canvas, int32 Width, int32 Height)
{
	if (!Canvas || !Model)
	{
		return;
	}
	hadi::MinimapView View;
	View.center = hadi::Vec2{static_cast<float>(State.Center.X), static_cast<float>(State.Center.Y)};
	View.yawDeg = State.CameraYaw;
	View.pixelsPerCm = State.PixelsPerCm;
	View.screenCenter = hadi::Vec2{Width * 0.5f, Height * 0.62f};
	const float CullRadius = FMath::Sqrt(static_cast<float>(Width * Width + Height * Height)) / State.PixelsPerCm;

	// Map shapes.
	for (const int32 Index : DrawOrder)
	{
		const hadi::MapRect& Rect = Model->map[static_cast<size_t>(Index)];
		if (hadi::Distance(Rect.center, View.center) - hadi::Max(Rect.half.x, Rect.half.y) > CullRadius)
		{
			continue;
		}
		const hadi::Vec2 Screen = hadi::WorldToMinimap(View, Rect.center);
		const float W = 2.0f * Rect.half.y * View.pixelsPerCm;
		const float H = 2.0f * Rect.half.x * View.pixelsPerCm;
		FCanvasTileItem Tile(FVector2D(Screen.x - 0.5f * W, Screen.y - 0.5f * H), FVector2D(W, H), HadiBridge::ToLinear(Rect.color));
		Tile.Rotation = FRotator(0.0f, hadi::MinimapRotation(View, Rect.yaw), 0.0f);
		Tile.PivotPoint = FVector2D(0.5f, 0.5f);
		Tile.BlendMode = SE_BLEND_Opaque;
		Canvas->DrawItem(Tile);
	}

	auto ToScreen = [&View](const FVector2D& World) {
		const hadi::Vec2 S = hadi::WorldToMinimap(View, hadi::Vec2{static_cast<float>(World.X), static_cast<float>(World.Y)});
		return FVector2D(S.x, S.y);
	};

	// GPS route.
	const FLinearColor RouteColor(0.75f, 0.25f, 1.0f, 1.0f);
	for (int32 Index = 1; Index < State.Route.Num(); ++Index)
	{
		FCanvasLineItem Line(ToScreen(State.Route[Index - 1]), ToScreen(State.Route[Index]));
		Line.SetColor(RouteColor);
		Line.LineThickness = 5.0f;
		Canvas->DrawItem(Line);
	}

	// Blips (clamped to the border when off-screen, like a radar).
	const FVector2D EdgeMin(10.0f, 10.0f);
	const FVector2D EdgeMax(Width - 10.0, Height - 10.0);
	for (const FHadiMinimapBlip& Blip : State.Blips)
	{
		FVector2D P = ToScreen(Blip.World);
		const bool bOutside = P.X < EdgeMin.X || P.Y < EdgeMin.Y || P.X > EdgeMax.X || P.Y > EdgeMax.Y;
		if (bOutside && !Blip.bClampToEdge)
		{
			continue;
		}
		P.X = FMath::Clamp(P.X, EdgeMin.X, EdgeMax.X);
		P.Y = FMath::Clamp(P.Y, EdgeMin.Y, EdgeMax.Y);
		const float Size = 18.0f;
		FCanvasTileItem Back(P - FVector2D(Size * 0.5f + 2.0f, Size * 0.5f + 2.0f), FVector2D(Size + 4.0f, Size + 4.0f), FLinearColor::Black);
		Back.BlendMode = SE_BLEND_Opaque;
		Canvas->DrawItem(Back);
		FCanvasTileItem Front(P - FVector2D(Size * 0.5f, Size * 0.5f), FVector2D(Size, Size), Blip.Color);
		Front.BlendMode = SE_BLEND_Opaque;
		Canvas->DrawItem(Front);
		if (Font && !Blip.Letter.IsEmpty())
		{
			float TextW = 0.0f;
			float TextH = 0.0f;
			Canvas->TextSize(Font, Blip.Letter, TextW, TextH, 0.6f, 0.6f);
			FCanvasTextItem Text(P - FVector2D(TextW * 0.5f, TextH * 0.5f), FText::FromString(Blip.Letter), Font, FLinearColor::Black);
			Text.Scale = FVector2D(0.6f, 0.6f);
			Canvas->DrawItem(Text);
		}
	}

	// North marker on the border.
	{
		const hadi::Vec2 NorthDir = hadi::WorldToMinimap(View, View.center + hadi::Vec2{100000.0f, 0.0f}) - View.screenCenter;
		const float Len = hadi::Length(NorthDir);
		if (Len > 0.0f && Font)
		{
			const FVector2D Dir(NorthDir.x / Len, NorthDir.y / Len);
			FVector2D P(View.screenCenter.x + Dir.X * 1000.0f, View.screenCenter.y + Dir.Y * 1000.0f);
			P.X = FMath::Clamp(P.X, 16.0, Width - 16.0);
			P.Y = FMath::Clamp(P.Y, 16.0, Height - 16.0);
			FCanvasTileItem Disc(P - FVector2D(13.0f, 13.0f), FVector2D(26.0f, 26.0f), FLinearColor(0.05f, 0.05f, 0.06f, 1.0f));
			Disc.BlendMode = SE_BLEND_Opaque;
			Canvas->DrawItem(Disc);
			float TextW = 0.0f;
			float TextH = 0.0f;
			Canvas->TextSize(Font, TEXT("N"), TextW, TextH, 0.7f, 0.7f);
			FCanvasTextItem Text(P - FVector2D(TextW * 0.5f, TextH * 0.5f), FText::FromString(TEXT("N")), Font, FLinearColor::White);
			Text.Scale = FVector2D(0.7f, 0.7f);
			Canvas->DrawItem(Text);
		}
	}

	// Player arrow (always at the minimap centre, pointing where Hadi faces).
	{
		const float Angle = FMath::DegreesToRadians(State.PlayerYaw - State.CameraYaw);
		const FVector2D C(View.screenCenter.x, View.screenCenter.y);
		const FVector2D Forward(FMath::Sin(Angle), -FMath::Cos(Angle));
		const FVector2D Right(-Forward.Y, Forward.X);
		const FVector2D Tip = C + Forward * 14.0f;
		const FVector2D BackLeft = C - Forward * 9.0f - Right * 9.0f;
		const FVector2D BackRight = C - Forward * 9.0f + Right * 9.0f;
		FCanvasTriangleItem Outline(Tip + Forward * 3.0f, BackLeft - Right * 2.5f - Forward * 2.0f, BackRight + Right * 2.5f - Forward * 2.0f, GWhiteTexture);
		Outline.SetColor(FLinearColor::Black);
		Canvas->DrawItem(Outline);
		FCanvasTriangleItem Arrow(Tip, BackLeft, BackRight, GWhiteTexture);
		Arrow.SetColor(FLinearColor::White);
		Canvas->DrawItem(Arrow);
	}
}
