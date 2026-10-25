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
    inline constexpr const char* noController = "Sin controladora";
    inline constexpr const char* audioPrefix = "Audio: ";
    inline constexpr const char* audioBuffer = " kHz · buffer ";
    inline constexpr const char* noAudioDevice = "Audio: sin dispositivo";
    inline constexpr const char* master = "Master";
    inline constexpr const char* record = "Grabar";
    inline constexpr const char* settings = "Ajustes";
    inline constexpr const char* library = "Biblioteca";
    inline constexpr const char* collections[] = { "Todas las pistas", "Carpetas", "Historial",
                                                   "Placas y drops" };
    inline constexpr const char* searchPlaceholder = "Buscar por título, artista, BPM o tono…";
    inline constexpr const char* libraryFilters[] = { "Todas las pistas", "BPM ±3 del deck A",
                                                      "Tono compatible con A", "Sin analizar" };
    inline constexpr const char* analyze = "Analizar";
    inline constexpr const char* importTracks = "Importar";
    inline constexpr const char* importTitle = "Importar pistas";
    inline constexpr const char* trackColumns[] = { "Deck", "Título", "Artista", "BPM", "Tono",
                                                    "Duración", "Cargar" };
    inline constexpr const char* addToAutoMix = "+";
    inline constexpr const char* autoMix = "Auto Mix";
    inline constexpr const char* pendingTracks = " pistas por sonar";
    inline constexpr const char* autoMixSwitch = "Auto Mix activo";
    inline constexpr const char* autoMixOn = "Activo";
    inline constexpr const char* autoMixOff = "Pausado";
    inline constexpr const char* collapseAutoMix = "Contraer el panel de Auto Mix";
    inline constexpr const char* expandAutoMix = "Mostrar el panel de Auto Mix";
    inline constexpr const char* autoMixStrip = "AUTO MIX · ";
    inline constexpr const char* autoMixMenus[] = { "Modo de decks", "Transición",
                                                    "Orden de reproducción", "Lista" };
    inline constexpr const char* deckModes[] = { "Alternar decks A/B", "Un solo deck" };
    inline constexpr const char* trimSilence = "Limpiar silencios";
    inline constexpr const char* matchBpm = "Emparejar BPM";
    inline constexpr const char* durationInBeats = "Duración en beats";
    inline constexpr const char* durationInSeconds = "Duración en segundos";
    inline constexpr const char* beats = " beats";
    inline constexpr const char* seconds = " segundos";
    inline constexpr const char* randomOrder = "Aleatorio";
    inline constexpr const char* shuffleList = "Revolver lista";
    inline constexpr const char* listActions[] = { "Limpiar lista", "Quitar tocadas",
                                                   "Quitar duplicadas" };
    inline constexpr const char* autoRemovePlayed = "Auto quitar tocadas";
    inline constexpr const char* saveList = "Guardar lista…";
    inline constexpr const char* loadList = "Cargar lista…";
    inline constexpr const char* summaryDecks[] = { "Decks A/B", "Un solo deck" };
    inline constexpr const char* summaryNormal = "Normal";
    inline constexpr const char* summaryTrim = "Sin silencios";
    inline constexpr const char* summaryBpm = "BPM emparejado";
    inline constexpr const char* summarySeconds = " s";
    inline constexpr const char* summaryAutoRemove = "Auto quitar";
    inline constexpr const char* separator = " · ";
    inline constexpr const char* queueStatuses[] = { "Tocada", "Sonando · A", "Siguiente · B",
                                                     "En cola" };
    inline constexpr const char* nextSingleDeck = "Siguiente · A (precarga)";
    inline constexpr const char* duplicate = "Duplicada";
    inline constexpr const char* bpmSuffix = " BPM";
    inline constexpr const char* removeFromQueue = "×";
    inline constexpr const char* emptyQueue =
        "La lista está vacía. Agrega pistas con el botón + de la biblioteca.";
}
