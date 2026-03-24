#pragma once

class Player;

//===============================
//		    GameSession
// 패킷 해석 및 Player 객체와 연결
//	   특정 한 판의 규칙과 상태
//===============================
class GameSession
{
public:
	GameSession();
	~GameSession();


private:
	int _id;

//	std::shared_ptr<Player> _player;

};

