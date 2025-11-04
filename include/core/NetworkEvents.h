#ifndef NETWORK_EVENTS_H
#define NETWORK_EVENTS_H

#include <cstdint>

// Network event code definitions
enum GameEventCode : uint8_t {
    EVENT_GAME_INIT = 1,           // Game initialization (deck seed, initial hand)
    EVENT_CARD_PLACED = 2,         // Card placed
    EVENT_DECK_CHOICE = 3,         // Deck choice (normal/special)
    EVENT_TURN_END = 4,            // Turn end
    EVENT_SPECIAL_CARD_START = 5,  // Special card usage start
    EVENT_SPECIAL_CARD_ACTION = 6, // Special card action
    EVENT_GAME_OVER = 7,           // Game over
    EVENT_PLAYER_READY = 8         // Player ready
};

// Event data structures
struct CardPlacedData {
    int32_t flagIndex;      // Target flag index
    int32_t slotIndex;      // Slot index (0-2)
    int32_t cardId;         // Card ID (color*10 + value)
    int32_t isSpecialCard;  // Is special card
    int32_t specialCardType; // Special card type
};

struct DeckChoiceData {
    int32_t deckType;       // 0: Normal deck, 1: Special deck
};

struct GameInitData {
    int32_t seed;           // Random seed
    int32_t hostPlayerIndex; // Host player index (0 or 1)
};

struct SpecialCardActionData {
    int32_t actionType;     // Action type
    int32_t param1;         // Parameter 1
    int32_t param2;         // Parameter 2
    int32_t param3;         // Parameter 3
};

#endif // NETWORK_EVENTS_H
