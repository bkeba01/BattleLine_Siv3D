#pragma once
enum ID_Player
{
	ste_Player1=0,
	ste_Player2=1,
	ste_PlayerMax=1,
	ste_PlayerMin = 0,
	ste_PlayerMakeNum=2,
	ste_NonPlayer=2
};
enum ID_Card
{
	ste_ColorMaxValue=6,
	ste_ColorMinValue = 1,
	ste_CardMaxValue = 10,
	ste_CardMinValue = 1,
	ste_SlotCardMaxNum=2,
	ste_SlotCardMinNum=0,
	ste_SlotCardMakeNum=3,
	ste_NonCard=0
};
enum ID_Role
{
	ste_RoleMaxNum=6,
	ste_StraightFlush=5,
	ste_ThreeOfKind=4,
	ste_Flush=3,
	ste_Straight=2,
	ste_NoneRole=1,
	ste_InitRole=0
};
