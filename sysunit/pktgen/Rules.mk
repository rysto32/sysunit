
LIB :=	pktgen
SRCS := \
	EtherAddr.cpp \
	EthernetMatcher.cpp \
	Ipv4Matcher.cpp \
	Ipv6Addr.cpp \
	Ipv6Matcher.cpp \
	Layer.cpp \
	PayloadMatcher.cpp \
	TcpMatcher.cpp \

TESTS := \
	EthernetHeader \
	Ipv4Header \
	Ipv6Header \
	TcpHeader \

MBUF_LIBS := \
	fake_mbuf \
	fake_atomic \
	fake_malloc \
	fake_mib \
	fake_panic \
	fake_uma \
	sysunit_init \

TEST_ETHERNETHEADER_SRCS := \
	EtherAddr.cpp \

TEST_ETHERNETHEADER_LIBS := \
	$(MBUF_LIBS) \

TEST_IPV4HEADER_LIBS := \
	$(MBUF_LIBS) \

TEST_IPV6HEADER_SRCS := \
	Ipv6Addr.cpp \

TEST_IPV6HEADER_LIBS := \
	$(MBUF_LIBS) \

TEST_TCPHEADER_LIBS := \
	$(MBUF_LIBS) \
