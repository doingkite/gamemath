#pragma once

namespace CK
{

struct HSVColor
{
public:
	FORCEINLINE constexpr HSVColor() = default;
    FORCEINLINE constexpr HSVColor(float InH, float InS, float InV) : H(InH), S(InS), V(InV) {}
    FORCEINLINE constexpr LinearColor ToLinearColor() const
	{
        float r = 0.f, g = 0.f, b = 0.f;

        //색상 바퀴를 6개의 구역(빨강, 노랑, 초록, 청록, 파랑, 보라)으로 나눈다
        int i = Math::FloorToInt(H * 6.f);

        // 각 구역에서 얼마나 치우쳐 있는가
        float f = H * 6.f - i;
        //채도 S 와 명도 V를 이용해 RGB의 중간값을 미리 만들어둔다.
        float p = V * (1.f - S);
        float q = V * (1 - f * S);
        float t = V * (1 - (1 - f) * S);

        switch (i % 6) {
        case 0: r = V, g = t, b = p; break; // 빨간색 영역에서 노란색으로 넘어가는 중
        case 1: r = q, g = V, b = p; break;
        case 2: r = p, g = V, b = t; break; // 초록
        case 3: r = p, g = q, b = V; break;
        case 4: r = t, g = p, b = V; break; //파랑
        case 5: r = V, g = p, b = q; break;
        }

        return LinearColor(r, g, b);
	}

public:
	float H = 0.f;
	float S = 1.f;
	float V = 1.f;
};

}

