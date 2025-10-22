#pragma once
enum ID_Player
{
	ste_Player1=0,
	ste_Player2=1,
	ste_PlayerMax=1,
	ste_PlayerMin = 0,
	ste_PlayerMakeNum=2,
	ste_NonePlayer=2
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
	ste_SlotCard_NonSpace = -1,
	ste_HandCardMakeNum = 7,
	ste_HandCardMaxNum = 6,
	ste_HandCardMinNum = 0,
	ste_HandCardFull=0,
	ste_NoneCard=0
	
};
enum ID_Deck
{
	ste_NoneDeck = -1
};
enum ID_Color
{
	ste_NoneColor = 0
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
enum ID_Flag
{
	ste_FlagMakeNum = 9,
	ste_FlagMaxNum=8,
	ste_FlagMinNum = 0,
	ste_FlagWinThreshold = 5,
	ste_FlagWinSerialThreshold = 3,
	ste_Flag_NonStatus = 0
};
enum SpecialCardType
{
	ste_Wildcard = 0,
	ste_StealCard = 1,
	ste_DestroyCard = 2,
	ste_SwapCard = 3,
	ste_DoubleValue = 4,
	ste_SpecialCardTypeCount = 5
};
enum ID_SpecialDeck
{
	ste_SpecialDeckSize = 10,
	ste_NoneSpecialCard = -1,
	ste_MaxSpecialCardsInHand = 3
};
