#pragma once

// Every UI text, in Spanish, so it can be translated later.
namespace strings
{
    inline constexpr const char* load = "Cargar";
    inline constexpr const char* play = "PLAY";
    inline constexpr const char* pause = "PAUSA";
    inline constexpr const char* cue = "CUE";
    inline constexpr const char* audioSettings = "Ajustes de audio";
    inline constexpr const char* chooseTrack = "Elegir pista";
    inline constexpr const char* loadFailed = "No se pudo cargar la pista";
    inline constexpr const char* noTrack = "Sin pista";
    inline constexpr const char* deckNames[] = { "Deck A", "Deck B" };
    inline constexpr const char* gain = "Gain";
    inline constexpr const char* eqHigh = "Agudos";
    inline constexpr const char* eqMid = "Medios";
    inline constexpr const char* eqLow = "Graves";
    inline constexpr const char* filter = "Filtro";
    inline constexpr const char* crossfader = "Crossfader";
    inline constexpr const char* crossfaderCurve = "Curva";
    inline constexpr const char* padModes[] = { "Hot cue", "Loop", "FX", "Placas" };
    inline constexpr const char* padEmpty = "Vacío";
    inline constexpr const char* loopSizes[] = { "1/8 beat", "1/4 beat", "1/2 beat", "1 beat",
                                                 "2 beat", "4 beat", "8 beat", "16 beat" };
    inline constexpr const char* deckLetters[] = { "A", "B" };
    inline constexpr const char* noValue = "—";
    inline constexpr const char* bpmAndKey = "BPM · Tono ";
    inline constexpr const char* sync = "SYNC";
    inline constexpr const char* keylock = "KEYLOCK";
    inline constexpr const char* loopButtons[] = { "1/2", "1", "2", "4", "8", "16" };
    inline constexpr const char* loopIn = "LOOP IN";
    inline constexpr const char* loopOut = "OUT";
    inline constexpr const char* reloop = "RELOOP";
    inline constexpr const char* pitch = "Pitch";
    inline constexpr const char* channelNames[] = { "Canal A", "Canal B" };
    inline constexpr const char* fx = "FX";
    inline constexpr const char* fxBeats[] = { "1/4", "1/2", "1", "2" };
    inline constexpr const char* fxTargets[] = { "→A", "→B" };
    inline constexpr const char* placas = "Placas";
    inline constexpr const char* fxNames[] = { "Echo", "Reverb", "Filtro", "Flanger",
                                               "Delay", "Phaser", "Gate", "Roll" };
}
