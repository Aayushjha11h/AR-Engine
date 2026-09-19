<img width="1439" height="758" alt="Screenshot 2026-09-19 170227" src="https://github.com/user-attachments/assets/b785d407-826c-4ebd-b2ca-b61df55d4289" />
<img width="1003" height="775" alt="Screenshot 2026-09-19 170157" src="https://github.com/user-attachments/assets/555b22e7-fa41-4685-b74a-c46ec03931ea" />
<img width="1004" height="789" alt="Screenshot 2026-09-19 170137" src="https://github.com/user-attachments/assets/29ce6b08-fc85-4f0c-8f9a-83a9d1811c21" />
<img width="992" height="764" alt="Screenshot 2026-09-19 163744" src="https://github.com/user-attachments/assets/e0f8ccad-a1a1-4496-84ce-8c9d7ddaeba6" />
# AR-Engine

A custom 2D game engine built with C++ and OpenGL, featuring a custom scripting language for game logic definition. The engine includes a complete physics system, rendering pipeline, and a demo Mario-style platformer game.

## Features

### Core Engine Systems
- **Physics System**: Rigid body dynamics with gravity, collision detection, impulse resolution, and friction
- **Rendering**: OpenGL-based rendering with sprite support, texture loading, and parallax backgrounds
- **Entity-Component Architecture**: Flexible component system for game objects
- **Custom Scripting Language (ARGDL)**: Human-readable game definition language for level design
- **Input Handling**: Keyboard and mouse input processing
- **Camera System**: Follow camera with smooth interpolation and zoom support
- **Audio System**: Sound playback with SDL2
- **Game State Management**: Title screen, playing, game over, and victory states

### Demo Game Features
The included demo game is a Mario-style platformer featuring:
- **Player Character**: Physics-based movement with jumping, gravity, and collision
- **12 Collectible Coins**: Scattered throughout the level for collection
- **5 Enemy Characters**: Boss sprites with physics and collision
- **Multiple Platform Types**: Ground platforms, walls, pipes, floating platforms, and stairs
- **Physics Objects**: Interactive crates with mass and restitution
- **Victory Condition**: Reach the flagpole at the end of the level
- **Death Zones**: Falling below platforms triggers game over
- **Sprite Graphics**: All game elements use texture sprites instead of solid colors

## Project Structure

```
ARengine/
├── engine/              # Core engine components
│   ├── Audio.cpp/h     # Audio system
│   ├── Camera.cpp/h    # Camera with follow and zoom
│   ├── Collision.cpp/h # Physics collision detection
│   ├── Component.cpp/h # Base component system
│   ├── Entity.cpp/h    # Entity management
│   ├── GameLoop.cpp/h  # Main game loop
│   ├── Gravity.cpp/h   # Gravity force application
│   ├── Input.cpp/h     # Input handling
│   ├── PatrolAI.cpp/h  # Enemy patrol AI
│   ├── BossAI.cpp/h    # Boss enemy AI
│   ├── Renderer.cpp/h  # OpenGL rendering
│   ├── RigidBody.cpp/h # Physics rigid bodies
│   ├── Scene.cpp/h     # Scene management
│   ├── Shader.cpp/h    # OpenGL shader management
│   ├── Sprite.h        # Sprite structure
│   ├── Texture.cpp/h   # Texture loading
│   ├── Timer.cpp/h     # Delta time calculation
│   └── Window.cpp/h    # Window management
├── Language/           # Custom scripting language
│   ├── Lexer.cpp/h     # Tokenizer for ARGDL
│   ├── Parser.cpp/h    # Parser for ARGDL
│   ├── Interpreter.cpp/h# Interpreter for ARGDL
│   ├── RuntimeBridge.cpp/h# Bridge between script and engine
│   └── Token.cpp/h     # Token definitions
├── ThirdParty/         # External dependencies
│   └── stb/           # stb_image for texture loading
├── assets/            # Game assets
│   ├── background.jpg  # Parallax background
│   ├── character.jpg  # Player sprite
│   ├── dirt.jpg       # Platform/ground texture
│   ├── coin.png       # Collectible coin sprite
│   ├── boss.png       # Enemy boss sprite
│   └── tree.png       # Scenery decoration
├── game.argdl         # Game level definition (ARGDL)
├── main.cpp           # Application entry point
└── README.md          # This file
```

## Building

### Requirements
- Visual Studio (2019 or later recommended)
- C++17 support
- OpenGL 3.3+
- SDL2
- GLEW
- GLM (OpenGL Mathematics)

### Build Steps
1. Open the provided `.vcxproj` file in Visual Studio
2. Ensure all dependencies are properly linked
3. Build the project in Debug or Release configuration
4. Run the executable from the build output directory

## Running the Game

1. Ensure the `assets/` folder is in the same directory as the executable
2. Run the compiled executable
3. Press **Enter** or **Space** on the title screen to start
4. Use **A/D** or **Left/Right** arrow keys to move
5. Use **W**, **Up arrow**, or **Space** to jump
6. Collect coins and reach the flagpole to win
7. Press **R** to restart after game over

## ARGDL Scripting Language

The engine uses a custom scripting language called ARGDL (AR Game Definition Language) for defining game levels. The language is human-readable and defines entities, their properties, and input handling.

### Entity Definition
```
#EntityName Type
{
    property : value
    property : value
}
```

### Supported Entity Types
- **Character**: Player-controlled characters
- **Platform**: Static or kinematic platforms
- **Item**: Collectible items (coins, power-ups)
- **Enemy**: Enemy characters with AI
- **Background**: Parallax background layers
- **Scenery**: Decorative elements

### Common Properties
- `x`, `y`: Position coordinates
- `width`, `height`: Dimensions (for platforms)
- `size`: Uniform size (for characters/items)
- `color`: Color name (red, green, blue, etc.)
- `sprite`: Path to texture file
- `speed`: Movement speed
- `jump_force`: Jump impulse strength
- `mass`: Physics mass
- `gravity`: Enable/disable gravity (on/off)
- `friction`: Surface friction coefficient
- `restitution`: Bounciness (0-1)
- `kinematic`: Enable kinematic physics (on/off)
- `collider`: Collision shape (box)
- `trigger`: Enable trigger collisions (on/off)
- `parallax`: Parallax scrolling factor
- `layer`: Render layer (background, scenery, main, foreground)

### Input Handling
```
on key
{
    command
}
```

### Supported Commands
- `move left/right/up/down`: Movement
- `jump`: Jump action
- `gravity on/off`: Toggle gravity
- `play sound_name`: Play sound effect

### Example Level Definition
```argdl
#Player Character
{
    x : 100
    y : 536
    size : 32
    color : red
    sprite : "assets/character.jpg"
    speed : 320
    jump_force : 500
    gravity : on
    collider : box
}

#Ground1 Platform
{
    x : 400
    y : 560
    width : 800
    height : 40
    color : green
    sprite : "assets/dirt.jpg"
    kinematic : on
    collider : box
}

#Coin1 Item
{
    x : 250
    y : 520
    size : 16
    sprite : "assets/coin.png"
    gravity : off
    trigger : on
    collider : box
}

on a
{
    move left
}

on d
{
    move right
}

on space
{
    jump
}
```

## Technical Details

### Physics System
- Rigid body dynamics with impulse-based collision resolution
- AABB (Axis-Aligned Bounding Box) collision detection
- Ground detection and friction
- Support for static, kinematic, and dynamic bodies
- Layer-based collision filtering

### Rendering Pipeline
- OpenGL 3.3 Core Profile
- Custom shader programs for sprite rendering
- Texture loading with stb_image
- Parallax background support
- Layer-based rendering order

### Architecture
- Entity-Component pattern for flexibility
- Component-based physics and rendering
- Script-driven game logic via ARGDL
- Clear separation between engine systems and game content

## Controls

- **A / Left Arrow**: Move left
- **D / Right Arrow**: Move right  
- **W / Up Arrow / Space**: Jump
- **R**: Restart (after game over)
- **Mouse Scroll**: Zoom camera
- **Enter / Space**: Start game (title screen)

## Game Mechanics

### Physics
- Gravity affects all entities with `gravity : on`
- Collision detection between entities and platforms
- Friction affects movement on surfaces
- Jump force determines jump height

### Gameplay
- Collect coins to increase score
- Avoid or jump on enemies to defeat them
- Reach the flagpole to complete the level
- Falling below platforms results in game over
- Camera follows the player smoothly

## License

This project is licensed under the [MIT License](LICENSE).

## Credits

Built with C++, OpenGL, SDL2, and custom game engine architecture.
