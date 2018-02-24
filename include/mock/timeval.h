
#ifndef MOCK_TIMEVAL_H
#define MOCK_TIMEVAL_H

#include "mock/GlobalMock.h"

class MockTimeval : public GlobalMockBase<MockTimeval>
{
public:
	MOCK_METHOD1(getmicrotime, void(struct timeval *));
};

class GlobalMockTimeval : public GlobalMock<MockTimeval>
{
public:
	void ExpectGetMicrotime(const struct timeval &tv)
	{
		EXPECT_CALL(**this, getmicrotime(testing::_))
		  .Times(1)
		  .WillOnce(testing::SetArgPointee<0>(tv))
		  .RetiresOnSaturation();
	}
};

#endif
