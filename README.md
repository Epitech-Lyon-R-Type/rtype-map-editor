# R-Type Map Editor

Éditeur de maps pour R-Type avec Raylib. Drag-and-drop d'assets, selection de backgrounds, contrôle de la longueur des maps, export/import JSON.

## Dépendances

### Fedora
```bash
sudo dnf install raylib-devel nlohmann-json-devel cmake gcc-c++
```

### Ubuntu/Debian
```bash
sudo apt install libraylib-dev nlohmann-json3-dev cmake g++
```

### macOS (Homebrew)
```bash
brew install raylib nlohmann-json cmake
```

## Build

```bash
git clone <repo-url>
cd rtype-map-editor
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

## Run

```bash
./build/rtype-map-editor
```

## Utilisation

### Navigation
- **Flèches gauche/droite** : scroll horizontal sur la map
- **DEL** : supprimer l'entité sélectionnée

### Placement d'entités
- Glisser-déposer un asset de la palette vers le canvas
- Cliquer sur une entité pour la sélectionner
- Glisser une entité sélectionnée pour la déplacer

### Configuration de la map
- **Backgrounds** : sélectionner un background dans le menu "Backgrounds"
- **Map Length** : augmenter/diminuer la longueur de la map (1-10x)

### Sauvegarde/Chargement
- **Ctrl+S** : sauvegarder la map (l'ID est extrait du nom du background)
- **Ctrl+O** : charger la dernière map sauvegardée

## Format JSON généré

```json
{
  "game": "./assets/configs/rtype.json",
  "map": {
    "id": 2,
    "scrollSpeed": 2.0,
    "width": 800,
    "height": 600
  },
  "waves": [
    {"x": 300, "y": 200, "name": "SKIMMER", "ref": 1}
  ]
}
```

## Architecture

- `src/main.cpp` : point d'entrée et chargement du config
- `src/map_editor.{hpp,cpp}` : logique principale avec Raylib
- `src/map_serializer.{hpp,cpp}` : sérialisation JSON
- `src/config_loader.{hpp,cpp}` : chargement des assets depuis client.json
- `src/entity_data.hpp` : structures de données
- `src/asset_info.hpp` : informations sur les assets

## Requirements

Le projet s'attend à trouver un fichier `client.json` contenant la configuration des sprites. Les chemins sont résolus automatiquement depuis :
- `../assets/configs/client.json` (depuis le build directory)
- `assets/configs/client.json` (depuis le répertoire courant)
- Autres emplacements possibles

## License

EPITECH (2026)

