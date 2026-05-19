# Faza 3 — Ściany wewnętrzne, Drzwi, Okna

## Zrealizowane funkcje

### Ściany wewnętrzne (`InnerWall`)
- Prostopadłościan 5 ścian (bez podłogi), renderowany shaderem Phong
- Tworzenie przez klikanie punktów w viewport (kursor IW)
- Kursor IW: niebieski kwadrat poruszający się przyciskami GÓRA/DÓŁ/LEWO/PRAWO
- Kursor na ścianie outlinu (żółty) — przyciski "Wzdłuż lewo/prawo" z automatycznym przejściem między ścianami
- Wizualne indykatory dostępności przycisków (szare = zablokowane)
- Geometryczne wycięcia na drzwi i okna (`buildWithCutouts`)
- Kolizje w trybie EXPLORE z uwzględnieniem otworów drzwiowych

### Drzwi (`Door`)
- Typ SWING (obrotowe) i SLIDING (przesuwne)
- Animacja otwierania gdy gracz zbliży się na 2m (tryb EXPLORE)
- Przejście — otwór bez skrzydła, zawsze otwarty
- Geometryczne wycięcia w ścianach obrysu i wewnętrznych
- Podświetlenie pozycji przed dodaniem (pomarańczowy prostokąt)
- Obsługa na ścianach obrysu i wewnętrznych

### Okna (`RoomWindow`)
- Rama (GL_LINES) + szyba (GL_TRIANGLES, alpha 0.3)
- Szyba pozycjonowana dokładnie w środku grubości ściany
- Geometryczne wycięcia w ścianach (parapet + nadproże)
- Podświetlenie pozycji przed dodaniem (błękitny prostokąt)
- Obsługa na ścianach obrysu i wewnętrznych

## Pliki

| Plik | Rola |
|---|---|
| `src/scene/InnerWall.h/.cpp` | Geometria + cutouty ścian wewnętrznych |
| `src/scene/Wall.h/.cpp` | Geometria ścian obrysu z wycięciami |
| `src/scene/Door.h/.cpp` | Drzwi z animacją |
| `src/scene/RoomWindow.h/.cpp` | Okna z ramą i szybą |
| `src/ui/Panel.h/.cpp` | ImGui panel, IWMoveFlags, zakładki |
| `src/main.cpp` | Logika sceny, kolizje, cutouty, GL |

## Wymagania PDF spełnione w tej fazie

- **Pkt 8 — Animacja:** drzwi otwierają się automatycznie (200°/s) po zbliżeniu gracza