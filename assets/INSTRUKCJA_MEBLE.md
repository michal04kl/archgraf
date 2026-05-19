# Jak dodać własne meble

## Wymagane pliki

### Model 3D (.obj)
Umieść plik `.obj` w folderze:
assets/models/krzeslo.obj
assets/models/stol.obj
assets/models/sofa.obj

### Tekstura (opcjonalna)
Umieść plik PNG lub JPG o **tej samej nazwie** co model:
assets/textures/krzeslo.png
assets/textures/stol.jpg
Program automatycznie znajdzie teksturę pasującą do modelu.

## Wymagania dotyczące modelu OBJ

- Format: standardowy Wavefront OBJ
- Trójkąty lub czworokąty (program automatycznie trianguluje)
- Obsługiwane dane: `v`, `vt`, `vn`, `f`
- Jeśli brak UV (`vt`) — model renderuje się jednolitym kolorem
- Jeśli brak normalnych (`vn`) — program oblicza normalne płaskie

## Skala

Model jest wczytywany w oryginalnej skali OBJ.
**Zalecana skala:** 1 jednostka OBJ = 1 metr.

Jeśli model jest za duży/mały, użyj suwaka "Skala" w panelu
lub przeskaluj model w Blenderze przed eksportem.

## Eksport z Blendera

1. Zaznacz obiekt
2. `File → Export → Wavefront (.obj)`
3. Opcje eksportu:
   - ✅ Write Normals
   - ✅ Include UVs
   - ✅ Triangulate Faces
   - Forward: `-Z Forward`
   - Up: `Y Up`
4. Zapisz do `assets/models/`

## Jak używać w programie

1. Wejdź w zakładkę **Meble** (tryb projektowania)
2. Wybierz model z listy
3. Ustaw obrót Y i skalę
4. Kliknij **"Postaw mebel"**
5. Kliknij w widoku TOP_DOWN aby ustawić pozycję
6. **R** = obrót o 45° podczas stawiania
7. **ESC** = anuluj stawianie

## Przykładowe darmowe modele OBJ

- https://free3d.com (filtruj: OBJ, darmowe)
- https://sketchfab.com (pobierz jako OBJ)
- https://clara.io