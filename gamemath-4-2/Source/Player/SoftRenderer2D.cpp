
#include "Precompiled.h"
#include "SoftRenderer.h"
#include <random>
using namespace CK::DD;

// 격자를 그리는 함수
void SoftRenderer::DrawGizmo2D()
{
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 그리드 색상
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// 뷰의 영역 계산
	Vector2 viewPos = g.GetMainCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// 좌측 하단에서부터 격자 그리기
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// 그리드가 시작되는 좌하단 좌표 값 계산
	Vector2 minPos = viewPos - extent;
	Vector2 minGridPos = Vector2(ceilf(minPos.X / (float)_Grid2DUnit), ceilf(minPos.Y / (float)_Grid2DUnit)) * (float)_Grid2DUnit;
	ScreenPoint gridBottomLeft = ScreenPoint::ToScreenCoordinate(_ScreenSize, minGridPos - viewPos);

	for (int ix = 0; ix < xGridCount; ++ix)
	{
		r.DrawFullVerticalLine(gridBottomLeft.X + ix * _Grid2DUnit, gridColor);
	}

	for (int iy = 0; iy < yGridCount; ++iy)
	{
		r.DrawFullHorizontalLine(gridBottomLeft.Y - iy * _Grid2DUnit, gridColor);
	}

	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	r.DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	r.DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}

// 게임 오브젝트 목록


// 최초 씬 로딩을 담당하는 함수
void SoftRenderer::LoadScene2D()
{
	// 최초 씬 로딩에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();

}

// 게임 로직과 렌더링 로직이 공유하는 변수
Vector2 currentPosition;
float currentScale = 10.f;

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float moveSpeed = 100.f;
	static float scaleMin = 5.f;
	static float scaleMax = 20.f;
	//애니메이션 왕복 시간
	static float duration = 1.5f;
	//현재 경과 시간 보관
	static float elapsedTime = 0.f;
	// static float scaleSpeed = 20.f;

	Vector2 inputVector = Vector2(input.GetAxis(InputAxis::XAxis), input.GetAxis(InputAxis::YAxis)).GetNormalize();
	Vector2 deltaPosition = inputVector * moveSpeed * InDeltaSeconds;
	// float deltaScale = input.GetAxis(InputAxis::ZAxis) * scaleSpeed * InDeltaSeconds;

	// 경과 시간과 sin 함수를 활용한 [0,1] 값 생성
	// 프레임 당 경과 시간을 elapsedTime에 더해 현재 경과 시간을 갱신
	elapsedTime += InDeltaSeconds;
	// 나머지 값을 계산하는 Mod 함수를 사용해 현재 경과 시간이 duration 값을 넘으면 0으로 초기화
	elapsedTime = Math::FMod(elapsedTime, duration);
	// 현재 시간에 대응되는 각을 계산, 왕복이 끝나고 다시 처음부터 시간하는 시간의 값은 durationdlsep 이에 대응되는 각의 값은 2 파이,
	// --> 현재 경과 시간 elapsedTime에 대응하는 각을 계산 후 currentRad에 저장
	float currentRad = (elapsedTime / duration) * Math::TwoPI;
	float alpha = (sinf(currentRad) + 1) * 0.5f;

	// 물체의 최종 상태 설정
	currentPosition += deltaPosition;
	// currentScale = Math::Clamp(currentScale + deltaScale, scaleMin, scaleMax);
	// 선형 보간 함수Lerp의 세 번째 인자에 0을 대입하면 최솟값 scaleMin을 얻고 1을 대입하면 scaleMax 값이 얻어지며 중간값 0.5를 대입하면 최솟값과 최댓값의 중간값을 얻게 됨.
	// alpha 값은 시간에 따라 [0,1] 범위를 왕복하므로 시간에 따라 최솟값과 최댓값 사이를 왕복하는 크기 값을 얻게 됨.
	currentScale = Math::Lerp(scaleMin, scaleMax, alpha);
}

// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 배경에 격자 그리기
	DrawGizmo2D();

	// 렌더링 로직의 로컬 변수
	float rad = 0.f;
	static float increment = 0.001f;
	static std::vector<Vector2> hearts;
	//HSV 모델을 따르는 각 0에 대응ㅎ하는 최초 색상을 지정
	HSVColor hsv(0.f,1.f, 0.85f);

	// 하트를 구성하는 점 생성
	if (hearts.empty())
	{
		for (rad = 0.f; rad < Math::TwoPI; rad += increment)
		{
			float sin = sinf(rad);
			float cos = cosf(rad);
			float cos2 = cosf(2 * rad);
			float cos3 = cosf(3 * rad);
			float cos4 = cosf(4 * rad);
			float x = 16.f * sin * sin * sin;
			float y = 13 * cos - 5 * cos2 - 2 * cos3 - cos4;
			hearts.push_back(Vector2(x, y));
		}
	}

	//값을 초기화한 후 색상을 증가시키며 점에 대응
	rad = 0.f;
	for (auto const& v : hearts)
	{
		//라디안 값인[0, 2pie] 를 [0,1]의 범위로 변경
		hsv.H = rad / Math::TwoPI;
		r.DrawPoint(v * currentScale + currentPosition, hsv.ToLinearColor());
		rad += increment;
	}

	// 현재 위치와 스케일을 화면에 출력
	r.PushStatisticText(std::string("Position : ") + currentPosition.ToString());
	r.PushStatisticText(std::string("Scale : ") + std::to_string(currentScale));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
