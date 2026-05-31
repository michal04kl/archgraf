# Faza 4 — Meble + Teksturowanie

## Zrealizowane funkcje

### Rendering (nowe klasy)
- `Mesh` — VAO/VBO, `upload()`, `draw()`, format `Vertex{position, normal, texCoord}`
- `Texture` — ładowanie PNG/JPG/BMP przez stb_image, bindowanie na unit
- `OBJLoader` — wczytywanie plików .obj (v, vt, vn, f; trójkąty i czworokąty)

### Biblioteka mebli wbudowanych (`FurnitureLibrary`)
19 typów procedural-mesh (prostopadłościany z kolorami Phong):

| Kategoria | Meble |
|---|---|
| Salon / Jadalnia | Krzesło, Stół, Kanapa, Fotel |
| Sypialnia | Łóżko, Szafa, Komoda |
| Kuchnia | Blat kuchenny, Komoda kuchenna, Szafka wisząca, Półka wisząca, Kuchenka, Lodówka, Zmywarka |
| Łazienka | Toaleta, Wanna, Prysznic, Umywalka, Pralka |

- Geometria generowana proceduralnie (bez plików zewnętrznych)
- Każdy mebel = zestaw parts z indywidualnymi kolorami
- Wisząca szafka (Y=1.4m) i półka (Y=1.5m) montowane automatycznie na właściwej wysokości

### System mebli (`FurnitureManager`)
- Tryb **wbudowany** — `addBuiltinItem()` (proceduralny, bez OBJ)
- Tryb **OBJ** — `addItem()` z cache'owaniem meshy i tekstur
- Auto-wykrywanie tekstur pasujących do modelu OBJ (ta sama nazwa, różne rozszerzenia)
- Renderowanie Phong z obsługą `useTexture` uniform

### Panel UI — zakładka Meble
- Biblioteka wbudowana z rozwijalnymi kategoriami
- Wybór obiektu → przycisk "Postaw" → klik LPM w widoku top-down
- Klawisz R — obrót o 45° podczas ustawiania
- ESC — anulowanie trybu ustawiania
- Sekcja modeli OBJ (automatycznie skanuje `assets/models/`)
- Lista postawionych mebli z przyciskiem usuwania

## Pliki

| Plik | Rola |
|---|---|
| `src/rendering/Mesh.h/.cpp` | Geometria GPU |
| `src/rendering/Texture.h/.cpp` | Ładowanie tekstur |
| `src/rendering/OBJLoader.h/.cpp` | Import .obj |
| `src/scene/FurnitureLibrary.h/.cpp` | 19 typów mebli proceduralnych |
| `src/scene/Furniture.h/.cpp` | Manager instancji (OBJ + procedural) |
| `src/ui/Panel.h/.cpp` | UI z zakładką Meble |
| `src/main.cpp` | Obsługa placement mode, rendering |
| `external/stb/stb_image.h` | Dekodowanie obrazków |
| `shaders/phong.frag` | Dodano `useTexture` + `diffuseTexture` |

---

## Wymagania PDF — stan po Fazie 4

| # | Wymaganie | Status |
|---|---|---|
| 1 | Inicjalizacja OpenGL + okno | ✅ Faza 0/1 |
| 2 | Min. 3 modele 3D | ✅ **Faza 4** — ściany + podłoga + 19 typów mebli |
| 3 | Rzutowanie perspektywiczne | ✅ Faza 1 |
| 4 | Interakcja mysz/klawiatura | ✅ Faza 1+2 |
| 5 | Oświetlenie ambient + directional | ✅ Faza 1 |
| 6 | Cieniowanie Phong | ✅ Faza 1 |
| 7 | Teksturowanie | ✅ **Faza 4** — Texture + OBJLoader + stb_image |
| 8 | Animacja | ✅ Faza 3 — drzwi otwierają się automatycznie |
| 9 | Komentarze + dokumentacja PDF | ⏳ Faza 6 |
| 10 | Filtrowanie obrazu | ⏳ Faza 6 |

**Wszystkie punkty obowiązkowe (1–8) są spełnione.**

---

## Plan pozostałych faz

### Faza 5 — Kreator obiektów + Import OBJ
- Okno ImGui do tworzenia brył (sześcian, kula, graniastosłup)
- Podgląd w czasie rzeczywistym
- `mergeToModel()` — scalanie brył w jeden obiekt
- Ulepszone wczytywanie OBJ (materiały MTL)
- Drag & drop mebli po scenie

### Faza 6 — Zapis / Undo / Postprocessing / Dokumentacja
- Serializacja sceny do JSON (`nlohmann/json`)
- `UndoRedo` — Ctrl+Z / Ctrl+Y
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
| Faza 5 | Kreator + Import OBJ | ⏳ DO ZROBIENIA |
| Faza 6 | Zapis/Undo/Postprocessing/Docs | ⏳ DO ZROBIENIA |

---

*Aby kontynuować: wklej ten plik i napisz "Jedziemy z fazą 5".*