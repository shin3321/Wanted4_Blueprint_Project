#pragma once

// 1. Windows 및 네트워크 관련 (순서 중요: winsock2가 windows보다 먼저 와야 함)
#include <winsock2.h>
#include <mswsock.h>
#include <Windows.h>

// 2. 표준 STL 컨테이너 및 자료구조
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// 3. 멀티스레딩 및 동기화
#include <atomic>
#include <mutex>
#include <thread>

// 4. 기타 유틸리티 및 입출력
#include <iostream>
#include <memory>
#include <cstring>

#include "99.Header/Protocol.h"
#include "99.Header/Vector2.h"
#include "99.Header/Types.h"
#include "02.Network/OverlappedEx.h"
#include "02.Network/RingBuffer.h"
#include "02.Network/TimerEvent.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

