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
	ste_SlotCardMaxNum=3,
	ste_SlotCardMinNum=0,
	ste_SlotCardMakeNum=4,
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
	ste_RoleMaxNum=7,
	ste_FourOfKind=6,
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
	ste_WildCard = 0,//リーダーカード、ワイルドカードでありPlayerごとに1枚しか使えない
	ste_WildCard_Eight = 1,//ワイルドカード8
	ste_WildCard_Shield = 2,//ワイルドカード盾
	ste_FogCard = 3,//霧-合計値で勝敗を決める
	ste_MudCard = 4,//泥-4枚で勝敗を決める
	ste_ReconCard = 5,//偵察カード-山札から3枚引き、手札から2枚山札に戻す
	ste_DeploymentCard = 6,//配置展開カード-自分のslotにあるカードを別のslotに移動、または削除
	ste_EscapeCard = 7,// 脱走カード-相手のslotにあるカードを削除
	ste_BetrayalCard = 8,// 裏切りカード-相手のslotにあるカードを自分のslotに移動
	ste_SpecialCardTypeCount = 9,

	ste_SpecialCardMakeNum = 10, //スペシャルカードの最大枚数
	ste_SpecialCardMaxNum=9,
	ste_SpecialCardMinNum = 0
};
enum SpecialCardCategory
{
	ste_TroopCard = 0,        // 部隊カード - スロットに配置
	ste_WeatherTacticCard = 1, // 気象戦術カード - フラグの下に配置
	ste_ConspiracyTacticCard = 2, // 謀略戦術カード - 特殊デッキの近くに配置
	 //特殊カードの
};
enum ID_SpecialDeck
{
	ste_SpecialDeckSize = 10,
	ste_NoneSpecialCard = -1,
	ste_MaxSpecialCardsInHand = 3
};
enum class DraggedCardType
{
	None,           // 何も持っていない
	NormalCard,     // 通常Card
	TroopCard,      // TroopCard (SpecialCard)
	WeatherCard,    // WeatherTacticCard (SpecialCard)
	ConspiracyCard  // ConspiracyTacticCard (SpecialCard)
};
