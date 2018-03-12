
LIB := netinet

TESTS := \
	tcp_lro \

TEST_TCP_LRO_SRCS := \
	tcp_lro.c \

TEST_TCP_LRO_LIBS := \
	fake_csum \
	fake_malloc \
	fake_mbuf \
	fake_atomic \
	fake_mib \
	fake_panic \
	fake_uma \
	fake_phash \
	mock_ifnet \
	mock_time \
	pktgen \
	sysunit_init \

TEST_TCP_LRO_STDLIBS := \
	gmock \
