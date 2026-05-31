# Faza 5 — Zaznaczanie, Przesuwanie i Edycja Mebli

## Zrealizowane funkcje

### Zaznaczanie mebli
- Kliknięcie LPM na meblu w widoku top-down → zaznaczenie
- Kliknięcie nazwy mebla na liście w panelu → zaznaczenie
- Żółty krzyżyk nad zaznaczonym meblem jako wskaźnik wizualny
- Kliknięcie w pustą przestrzeń → odznaczenie

### Przesuwanie
- Przycisk "Przesuń" w panelu po zaznaczeniu mebla
- Zielony krzyżyk na kursorze podczas trybu przesuwania
- Kliknięcie LPM w nowe miejsce → mebel się przesuwa
- ESC anuluje tryb przesuwania
- Overlay informacyjny w widoku top-down

### Edycja obrotu
- Suwak "Obrót Y" (0–360°) dla zaznaczonego mebla
- Zmiana obrotu w czasie rzeczywistym bez konieczności usuwania i stawiania od nowa

### Usuwanie
- Przycisk X przy każdym meblu na liście
- Usuwa konkretny mebel zachowując indeksy pozostałych
- Po usunięciu zaznaczonego — selekcja resetuje się

### Lista mebli
- Każda pozycja pokazuje nazwę: "[0] Krzeslo", "[1] Kanapa" itd.
- Wyświetlana pozycja XZ zaznaczonego mebla
- Przycisk "Odznacz" do ręcznego kasowania selekcji

## Sterowanie

| Akcja | Sposób |
|---|---|
| Zaznacz mebel | LPM na meblu w widoku top-down |
| Zaznacz z listy | Kliknij nazwę w panelu |
| Przesuń | Zaznacz → "Przesuń" → LPM w nowe miejsce |
| Obróć | Zaznacz → suwak Obrót Y |
| Usuń | Przycisk X na liście |
| Anuluj przesuwanie | ESC |

## Zmodyfikowane pliki

| Plik | Zmiany |
|---|---|
| `src/ui/Panel.h` | Nowe pola w PanelResult: `clickedFurnitureIdx`, `requestMoveFurniture`, `newFurnitureRotY`; nowe parametry `render()` |
| `src/ui/Panel.cpp` | Nowa sekcja zaznaczonego mebla, lista z nazwami, suwak obrotu, przycisk Przesuń |
| `src/main.cpp` | Globale `selectedFurnitureIdx`, `furnitureMoveMode`; logika selekcji viewport, przesuwania, edycji obrotu, usuwania z korekcją indeksów |

---

## Wymagania PDF — stan po Fazie 5

| # | Wymaganie | Status |
|---|---|---|
| 1 | Inicjalizacja OpenGL + okno | ✅ Faza 0/1 |
| 2 | Min. 3 modele 3D | ✅ Faza 4 |
| 3 | Rzutowanie perspektywiczne | ✅ Faza 1 |
| 4 | Interakcja mysz/klawiatura | ✅ Faza 1+2 |
| 5 | Oświetlenie ambient + directional | ✅ Faza 1 |
| 6 | Cieniowanie Phong | ✅ Faza 1 |
| 7 | Teksturowanie | ✅ Faza 4 |
| 8 | Animacja | ✅ Faza 3 |
| 9 | Komentarze + dokumentacja PDF | ⏳ Faza 6 |
| 10 | Filtrowanie obrazu | ⏳ Faza 6 |

**Wszystkie punkty obowiązkowe (1–8) spełnione. Pozostaje tylko Faza 6.**

---

## Plan Fazy 6

### Faza 6 — Zapis / Undo / Postprocessing / Dokumentacja
- Serializacja sceny do JSON (`nlohmann/json`) — zapis i wczytywanie projektów
- `UndoRedo` — Ctrl+Z / Ctrl+Y dla wszystkich operacji
- `PostProcessor` — FBO → shader filtrujący (blur, sharpen) — **pkt 10 PDF**
- Komentarze w kodzie — **pkt 9 PDF**
- Dokumentacja PDF projektu

---

## STATUS FAZ

| Faza | Nazwa | Status |
|---|---|---|
| Faza 0 | Setup środowiska | ✅ GOTOWE |
| Faza 1 | Renderer + Kamera + Shadery Phong | ✅ GOTOWE |
| Faza 2 | Obrys mieszkania + ImGui | ✅ GOTOWE |
| Faza 3 | Ściany wew. + Drzwi + Okna | ✅ GOTOWE |
| Faza 4 | Meble + Tekstury | ✅ GOTOWE |
| Faza 5 | Zaznaczanie + Przesuwanie + Edycja mebli | ✅ GOTOWE |
| Faza 6 | Zapis/Undo/Postprocessing/Docs | ⏳ DO ZROBIENIA |

---

*Aby kontynuować: wklej ten plik i napisz "Jedziemy z fazą 6".*