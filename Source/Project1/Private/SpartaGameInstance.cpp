

#include "SpartaGameInstance.h"

USpartaGameInstance::USpartaGameInstance()
{
	TotalScore = 0;
	CurrentLevelIndex = 0;
	CurrentRecord = 0;
}

void USpartaGameInstance::AddToScore(int32 Amount)
{
	TotalScore += Amount;
}

// 멤버변수인 TotalScore 대신 매개변수 Score의 값으로 기록 점수 업데이트
// (값을 받지 않으면 Score = 0 기본값으로 되어 Getter 역할을 하므로 선택적으로 업데이트 가능)
int32 USpartaGameInstance::UpdateAndGetRecord(int32 Score)
{
	if (Score > CurrentRecord)
	{
		CurrentRecord = Score;
	}

	return CurrentRecord;
}
