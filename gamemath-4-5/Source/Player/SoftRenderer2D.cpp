
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
float currentDegree = 0.f;

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float rotateSpeed = 180.f;

	float deltaDegree = input.GetAxis(InputAxis::WAxis) * rotateSpeed * InDeltaSeconds;

	// 물체의 최종 상태 설정
	currentDegree += deltaDegree;
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
	// 사각형의 절반 크기 저장
	static float halfSize = 100.f;
	// 정사각형을 구성하는 벡터를 담는 컨테이너 선언
	static std::vector<Vector2> squares;

	// 사각형을 구성하는 점을 생성
	if (squares.empty())
	{
		// 밀도를 위해 촘촘히 점 생성
		for (float x = -halfSize; x <= halfSize; x += 0.25f)
		{
			for (float y = -halfSize; y <= halfSize; y += 0.25f)
			{
				squares.push_back(Vector2(x, y));
			}
		}
	}
	// 각도에 해당하는 sin과 cos 함수 얻기
	float sin=0.f; float cos = 0.f;
	Math::GetSinCosRad(sin, cos, currentDegree);

	// 현재 화면의 크기로부터 길이를 비교할 기준양 정하기
	// 길이에 따라 회전량을 다르게 부여하기 위해 기준 값을 정함. 화면 대각선 크기의 절반을 기준 값으로 변수에 저장
	static float maxLength = Vector2(_ScreenSize.X, _ScreenSize.Y).Size() * 0.5f;

	// 원을 구성하는 사각형 그리기
	HSVColor hsv(0.f, 1.f, 0.85f);
	for (auto const& v : squares)
	{
		// 극 좌표계로 변환
		Vector2 polarV = v.ToPolarCoordinate();

		// 극좌표계의 각은 [-파이, 파이]의 범위를 가진다. 이를 양수인 [0, 2파이]로 범위를 변경하기 위해 
		// 만약 각도가 음수라면
		if (polarV.Y < 0.f)
		{
			// 한 바퀴 2파이를 더해서 양수로 바꿔
			polarV.Y += Math::TwoPI;
		}
		// 모든 각도가 [0, 2파이] 안에 들어왔다면 2파이로 나눠서 0.0-1.0 사이의 비율로 출력 정규화
		hsv.H = polarV.Y / Math::TwoPI;

		// 극좌표계의 크기 정보로부터 회전량 결정
		// 해당 벡터의 길이와 maxLength와의 비를 구하고 변수 ratio에 저장
		float ratio = polarV.X / maxLength;
		// 선형 보간 함수 Lerp에 변수 ratio 값을 사용해 최소 1에서 5까지의 가중치 값을 구하고 weight에 저장
		float weight = Math::Lerp(1.f, 5.f, ratio);

		// 극좌표계를 이용한 회전 부여
		// 회전량에 가중치 weight을 곱해 최종 회전량을 계산해 회전을 부여, 중심에서 바깥쪽으로 갈수로 많은 가중치 부여
		polarV.Y += Math::Deg2Rad(currentDegree) * weight;

		// 최종 값을 다시 데카르트로 변환
		Vector2 cartesianV = polarV.ToCartesianCoordinate();
		r.DrawPoint(cartesianV, hsv.ToLinearColor());
	}

	// 현재 각도를 화면에 출력
	r.PushStatisticText(std::string("Degree : ") + std::to_string(currentDegree));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
