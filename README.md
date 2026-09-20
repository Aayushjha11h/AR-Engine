<img width="1439" height="758" alt="Screenshot 2026-09-19 170227" src="https://github.com/user-attachments/assets/b785d407-826c-4ebd-b2ca-b61df55d4289" />
<img width="1003" height="775" alt="Screenshot 2026-09-19 170157" src="https://github.com/user-attachments/assets/555b22e7-fa41-4685-b74a-c46ec03931ea" />
<img width="1004" height="789" alt="Screenshot 2026-09-19 170137" src="https://github.com/user-attachments/assets/29ce6b08-fc85-4f0c-8f9a-83a9d1811c21" />
<img width="992" height="764" alt="Screenshot 2026-09-19 163744" src="https://github.com/user-attachments/assets/e0f8ccad-a1a1-4496-84ce-8c9d7ddaeba6" />

# AR-Engine

A custom 2D game engine built from scratch with C++ and OpenGL, featuring its own scripting language, **ARGDL** (AR Game Definition Language), for describing levels, entities, and game logic. The engine includes a complete physics system, rendering pipeline, combat/AI systems, and demo games, including a Mario-style platformer.

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Building](#building)
- [Running the Game](#running-the-game)
- [ARGDL Scripting Language](#argdl-scripting-language)
  - [Design Goals](#design-goals)
  - [How a Script Becomes a Game](#how-a-script-becomes-a-game)
  - [Language Overview](#language-overview)
  - [Formal Grammar](#formal-grammar)
  - [Entity Blocks](#entity-blocks)
  - [Entity Types](#entity-types)
  - [Property Reference](#property-reference)
  - [Prefabs](#prefabs)
  - [Event Handlers](#event-handlers)
  - [Command Reference](#command-reference)
  - [Targets and Matching](#targets-and-matching)
  - [Coordinate System](#coordinate-system)
  - [Walkthrough: A Minimal Level](#walkthrough-a-minimal-level)
  - [Recipes](#recipes)
  - [The Bundled Example Games](#the-bundled-example-games)
  - [Tips and Gotchas](#tips-and-gotchas)
- [Technical Details](#technical-details)
- [Controls](#controls)
- [Game Mechanics](#game-mechanics)
- [License](#license)
- [Credits](#credits)

## Features

### Core Engine Systems
- **Physics System**: Rigid body dynamics with gravity, collision detection, impulse resolution, and friction
- **Rendering**: OpenGL-based rendering with sprite support, texture loading, and parallax backgrounds
- **Entity-Component Architecture**: Flexible component system for game objects
- **Custom Scripting Language (ARGDL)**: Human-readable game definition language for level design, input bindings, collision rules, timers, and spawning
- **Input Handling**: Keyboard and mouse input processing
- **Camera System**: Follow camera with smooth interpolation and zoom support
- **Audio System**: Sound playback with SDL2
- **Game State Management**: Title screen, playing, game over, and victory states
- **Combat and AI**: Health, damage, invulnerability frames, knockback, projectiles, patrolling enemies, boss enemies, and timed spawners, all configurable from ARGDL

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

Two additional ARGDL levels show off the combat side of the engine: shooting, turrets, patrolling enemies, spawners, falling meteors, a timed boss, and a portal victory condition (see [The Bundled Example Games](#the-bundled-example-games)).

## Project Structure

```
ARengine/
├── engine/                # Core engine components
│   ├── Audio.cpp/h        # Audio system management
│   ├── BossAI.cpp/h       # Boss enemy behavior and AI logic
│   ├── Camera.cpp/h       # Camera with follow and zoom capabilities
│   ├── Collision.cpp/h    # Physics collision detection and response
│   ├── Component.cpp/h    # Base component system
│   ├── Entity.cpp/h       # Entity lifecycle and management
│   ├── GameLoop.cpp/h     # Main application game loop
│   ├── Gravity.cpp/h      # Gravity force application
│   ├── Health.cpp/h       # Entity health and damage tracking
│   ├── Input.cpp/h        # Keyboard and input event handling
│   ├── PatrolAI.cpp/h     # Enemy patrol AI behavior
│   ├── Renderer.cpp/h     # OpenGL 2D batch/quad rendering
│   ├── RigidBody.cpp/h    # Physics rigid bodies and forces
│   ├── Scene.cpp/h        # Scene tree and level management
│   ├── Shader.cpp/h       # OpenGL shader compiling and management
│   ├── Sound.cpp/h        # Sound effect playback and instances
│   ├── Spawner.cpp/h      # Dynamic entity spawner logic
│   ├── Sprite.h           # Sprite and vertex structures
│   ├── Texture.cpp/h      # Texture loading and binding
│   ├── Timer.cpp/h        # Frame timing and delta time calculation
│   └── Window.cpp/h       # SDL Window creation and OpenGL context
├── Language/              # Custom scripting language (ARGDL)
│   ├── Lexer.cpp/h        # Tokenizer for ARGDL scripts
│   ├── Parser.cpp/h       # AST Parser for ARGDL scripts
│   ├── Interpreter.cpp/h # Interpreter for ARGDL execution
│   ├── RuntimeBridge.cpp/h # Bridge connecting ARGDL to C++ engine core
│   └── Token.cpp/h        # Lexical token definitions
├── ThirdParty/            # External dependencies
│   └── stb/               # stb_image for image texture decoding
├── assets/                # Game graphics assets
├── game.argdl             # Main game level script (ARGDL)
├── game2.argdl            # Demo game script 2
├── game3.argdl            # Shooter platformer demo game script 3
├── main.cpp               # Application entry point
└── README.md              # Project documentation
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

---

## ARGDL Scripting Language

ARGDL (**AR Game Definition Language**) is a small, declarative domain-specific language (DSL) that describes an entire game: what exists in the world, how it looks and behaves physically, and what happens when the player presses keys, things collide, or time passes.

Instead of recompiling C++ every time you move a platform or change enemy health, you edit a plain-text `.argdl` file and re-run the game.

A script contains exactly two kinds of top-level construct:

| Construct | Starts with | Purpose |
|---|---|---|
| **Entity block** | `#` | Declares *something in the game*: a player, platform, coin, enemy, background, or a reusable prefab template |
| **Event handler** | `on` or `after` | Declares *what happens when* a key is pressed, two things collide, or a timer fires |

Everything is **data-first**: entity blocks are lists of `property : value` pairs, and handlers are short lists of commands.

### Design Goals

- **Readable**: a level file reads like a description of the level.
- **Declarative**: you say *what* exists and *what should happen*, not how to loop, allocate, or render.
- **Engine-native**: every property maps directly onto an engine component (RigidBody, Collision, Sprite, PatrolAI, BossAI, and so on).
- **Iterable**: change a number, restart, see the result.

### How a Script Becomes a Game

```
 game.argdl  ─►  Lexer  ─►  Parser  ─►  Interpreter  ─►  RuntimeBridge  ─►  Engine
 (text)         (tokens)    (syntax     (executes the     (creates entities,   (physics,
                             tree)       program)          components,          rendering,
                                                            event bindings)      audio, ...)
```

1. **Lexer** (`Language/Lexer.cpp`) turns raw text into tokens (`#`, names, `{`, `}`, `:`, numbers, strings, keywords).
2. **Parser** (`Language/Parser.cpp`) checks the tokens against the grammar and builds a structured representation of entity blocks and event handlers.
3. **Interpreter** (`Language/Interpreter.cpp`) walks that structure: entity blocks are instantiated, and event handlers are registered so they can run later.
4. **RuntimeBridge** (`Language/RuntimeBridge.cpp`) is the only layer that touches the engine. It converts script-level concepts (a `Platform` with `kinematic : on`, a `spawn` command) into engine-level objects and calls (`Entity`, `RigidBody`, `Scene`, `Audio`, ...).

Keeping the bridge separate means the language can evolve without the engine depending on script syntax, and vice versa.

### Language Overview

A tiny script showing all the major pieces:

```argdl
#Player Character            // <- entity block: name = Player, type = Character
{
    x : 100                  // <- property : value
    y : 500
    size : 32
    sprite : "assets/character.jpg"
    gravity : on
    collider : box
}

on d                         // <- event handler: when key D is held/pressed
{
    move right               // <- command
}

on collide Coin              // <- event handler: when something collides with a Coin
{
    despawn
    play pickup
}
```

Basic lexical rules, as used throughout the bundled scripts:

- **Whitespace and line breaks** are not significant; one property per line is the convention.
- **Numbers** may be integers or decimals (`320`, `0.5`, `1.5`).
- **Booleans** are the words `on` and `off`.
- **Strings** are double-quoted and used for file paths: `"assets/coin.png"`.
- **Bare words** are used for colors (`red`, `cyan`), collider shapes (`box`), tags (`Turret`), and names (`GruntPrefab`).
- **Names are case-sensitive.** `Player` and `player` are different names.

### Formal Grammar

An informal EBNF-style description of the language:

```
program      = { entity | handler } ;

entity       = "#" Name Type "{" { property } "}" ;
property     = key ":" value ;
value        = number | "on" | "off" | word | string ;

handler      = "on" trigger "{" { command } "}"
             | "after" number "{" { command } "}" ;

trigger      = key                       (* a, d, space, left, j, ... *)
             | "collide" Target          (* on collide Coin           *)
             | "timer" number ;          (* on timer 5                *)

command      = "move" ("left" | "right" | "up" | "down")
             | "jump"
             | "rotate" number
             | "gravity" ("on" | "off")
             | "play" word
             | "spawn" Name [ "at" number number ]
             | Name "." "spawn" Name
             | "despawn"
             | "heal" Target number
             | "damage" [ Target ] number
             | "look_at" Name Target
             | "victory" ;
```

### Entity Blocks

```
#Name Type
{
    property : value
    property : value
}
```

- `#Name` is the entity's **unique name**. Other parts of the script refer to it by this name (for example `TurretLeft.spawn ...` or `look_at TurretLeft Player`).
- `Type` chooses the entity's role and default behavior.
- The braces contain any number of `property : value` lines. Properties you leave out fall back to engine defaults.

A common naming convention is a **shared prefix plus a number** for repeated things: `Coin1`, `Coin2`, `Ground3`, `Enemy4`. Handlers can then match the whole family with one word (see [Targets and Matching](#targets-and-matching)).

### Entity Types

| Type | Used for | Notes |
|---|---|---|
| `Character` | The player-controlled character | Typically has `speed`, `jump_force`, `mass`, and (in combat levels) `health`, `invuln_duration`, `knockback` |
| `Platform` | Ground, walls, pipes, stairs, floating platforms | Usually `kinematic : on` so it is solid but never moves. Sized with `width` and `height` |
| `Item` | Coins, crates, blocks, turrets, spawners, portals | A general-purpose "thing"; its behavior comes from its properties (`trigger`, `kinematic`, `tag`, `spawn_every`, ...) |
| `Enemy` | Enemies | Add `patrol_min`, `patrol_max`, `patrol_speed` for patrol AI, or `boss : on` for boss AI |
| `Background` | Parallax background layer | Usually `skip_collision : on` and a small `parallax` factor |
| `Scenery` | Decorative elements (for example trees) | Purely visual |
| `Prefab` | A **template** that is not placed in the world until spawned | Has no `x`/`y`; see [Prefabs](#prefabs) |

### Property Reference

#### Position and size

| Property | Value | Meaning |
|---|---|---|
| `x`, `y` | number | Position of the entity's **center**, in world units |
| `width`, `height` | number | Dimensions, used for platforms and backgrounds |
| `size` | number | Uniform size for square-ish things (characters, items, enemies, projectiles) |

#### Appearance

| Property | Value | Meaning |
|---|---|---|
| `color` | color name | Tint / fallback color (`red`, `green`, `blue`, `cyan`, `yellow`, `purple`, `gray`, `brown`, `orange`, `magenta`, ...) |
| `sprite` | `"path"` | Texture file, relative to the working directory (for example `"assets/coin.png"`) |
| `layer` | `background`, `scenery`, `main`, `foreground` | Render order layer |
| `parallax` | number | Background scroll factor. `0.1` scrolls at 10% of camera speed, giving depth |

#### Movement and physics

| Property | Value | Meaning |
|---|---|---|
| `speed` | number | Horizontal movement speed used by `move` |
| `jump_force` | number | Impulse applied by `jump` |
| `mass` | number | Physics mass. Large values (for example `999`) make an object effectively immovable in collisions |
| `gravity` | `on` / `off` | Whether gravity affects this entity |
| `friction` | number | Surface friction coefficient |
| `restitution` | 0 to 1 | Bounciness |
| `kinematic` | `on` / `off` | Solid but not moved by forces. Ideal for platforms and turrets |
| `physics` | `on` / `off` | `off` skips physics simulation entirely (used for bullets and other projectiles) |

#### Collision

| Property | Value | Meaning |
|---|---|---|
| `collider` | `box` | Collision shape (AABB) |
| `trigger` | `on` / `off` | Detects overlap and fires `on collide` handlers but produces **no physical response**. Used for coins and portals |
| `skip_collision` | `on` / `off` | Excludes the entity from collision entirely (backgrounds, spawners) |
| `tag` | word | A label that `on collide` handlers and projectiles can match (`Turret`, `Enemy`, `Portal`, `Meteor`, ...) |

#### Health and combat

| Property | Value | Meaning |
|---|---|---|
| `health` | number | Hit points. The entity is removed (or, for the player, the game ends) when this reaches zero |
| `invuln_duration` | seconds | How long the entity is invulnerable after taking a hit |
| `knockback` | number | Push applied when the entity is hurt |
| `hp` | number | Hit points used by boss entities (used together with `boss : on`) |
| `boss` | `on` / `off` | Attach boss AI |

#### Projectiles (for prefabs that get spawned as bullets or hazards)

| Property | Value | Meaning |
|---|---|---|
| `projectile_speed` | number | Travel speed |
| `projectile_lifetime` | seconds | Time before the projectile disappears |
| `projectile_damage` | number | Damage dealt on hit |
| `projectile_angle` | degrees | Fixed travel direction. Because +y points down, `90` is straight down. If omitted, the projectile travels in the direction the shooter is facing (set by `rotate` or `look_at`) |
| `projectile_friendly_tag` | tag | A tag this projectile will not harm (for example meteors ignoring `Enemy`) |

#### AI

| Property | Value | Meaning |
|---|---|---|
| `patrol_min`, `patrol_max` | x coordinates | Left and right limits of a patrol route |
| `patrol_speed` | number | Walking speed while patrolling |

#### Spawners

| Property | Value | Meaning |
|---|---|---|
| `spawn_every` | seconds | Interval between automatic spawns |
| `spawn_prefab` | Prefab name | Which prefab to spawn |
| `spawn_at_self` | `on` / `off` | Spawn at the spawner's own position |

### Prefabs

A **Prefab** is an entity template. It is declared with the normal block syntax but is **not placed in the world**. It sits ready until a `spawn` command (from a handler or a spawner) creates a copy.

```argdl
#PlayerBulletPrefab Prefab
{
    tag : PlayerBullet
    size : 12
    sprite : "assets/laserBlue01.png"
    physics : off
    collider : box
    projectile_speed : 900
    projectile_lifetime : 0.6
    projectile_damage : 1
}

on j
{
    spawn PlayerBulletPrefab      // fire a bullet when J is pressed
}
```

Prefabs are how you build anything that appears at runtime: bullets, meteors, wave enemies, bosses, and even spawners themselves (`ExtraSpawnerPrefab` in the meteor level).

### Event Handlers

Handlers bind a **trigger** to a list of **commands**.

```
on <trigger>
{
    command
    command
}

after <seconds>
{
    command
}
```

| Trigger | Example | Fires when |
|---|---|---|
| Key name | `on a`, `on space`, `on left`, `on j` | The key is pressed / held. Letter keys, `space`, and the arrow keys (`left`, `right`, `up`, `down`) are used in the bundled scripts |
| Collision | `on collide Coin` | The player collides with something matching the target |
| Repeating timer | `on timer 5` | Every 5 seconds, repeatedly |
| One-shot delay | `after 25` | Once, 25 seconds after the level starts |

Notes:

- Several handlers can bind the same key (`on a` and `on left` both move left), so you can offer multiple control schemes.
- Several timers can share an interval or coexist (`on timer 5` appears twice in the meteor level, once for a turret and once for a meteor).
- A handler may contain any number of commands, which run top to bottom.

### Command Reference

| Command | Example | Effect |
|---|---|---|
| `move <dir>` | `move left` | Move the player in `left`, `right`, `up`, or `down` at its `speed` |
| `jump` | `jump` | Apply the player's `jump_force` impulse (works when grounded) |
| `rotate <degrees>` | `rotate 180` | Set the player's facing direction. `0` faces right, `180` faces left. Used to point bullets and flip the sprite |
| `gravity on/off` | `gravity off` | Toggle gravity |
| `play <sound>` | `play pickup` | Play a named sound effect |
| `spawn <Prefab>` | `spawn PlayerBulletPrefab` | Create a copy of a prefab at the acting entity |
| `spawn <Prefab> at <x> <y>` | `spawn BossPrefab at 600 100` | Create a copy of a prefab at a world position |
| `<Entity>.spawn <Prefab>` | `TurretLeft.spawn EnemyBulletPrefab` | Make a **specific named entity** spawn the prefab (so a turret fires from the turret) |
| `despawn` | `despawn` | Remove the entity involved in the event (for example the coin that was touched) |
| `heal <Target> <n>` | `heal Player 2` | Restore `n` health to the target |
| `damage <Target> <n>` | `damage Player 1` | Deal `n` damage to the target |
| `damage <n>` | `damage 1` | Deal `n` damage to the other entity involved in the collision |
| `look_at <Entity> <Target>` | `look_at TurretLeft Player` | Rotate an entity to face another, used to aim turrets |
| `victory` | `victory` | End the level in the victory state |

### Targets and Matching

`on collide X`, `heal X`, `damage X`, and `look_at A X` all take a **target**. In the bundled scripts a target is matched against an entity's **name**, **type**, or **tag**. That makes it easy to write one rule for a whole family:

```argdl
on collide Coin        // matches Coin1, Coin2, ... Coin12 by their shared name prefix
{
    despawn
    heal Player 2
}

on collide Enemy       // matches every Enemy: patrolling grunts, bosses, spawned grunts
{
    damage Player 1
}

on collide Turret      // matches anything with  tag : Turret
{
    damage Player 1
}
```

Use `#Name` to identify **one** entity (`TurretLeft`), and `tag` to group **different** entities under one label (both `TurretLeft` and `TurretRight` share `tag : Turret`).

### Coordinate System

- Coordinates are in world units, with the origin at the **top-left** of the world.
- **+x goes right, +y goes down.** That is why the ground in the platformer sits at `y : 560` while the sky is near `y : 0`, and why falling meteors spawn at negative `y` (above the screen) with `projectile_angle : 90`.
- `x` and `y` are the entity's **center**. A platform with `x : 400`, `width : 800` covers `x = 0` to `800`.
- Backgrounds are usually larger than the screen and centered in the world, for example `x : 1500, width : 3000` in the platformer.

### Walkthrough: A Minimal Level

Here is a complete, tiny, playable level with a floor, a player, a coin, and controls:

```argdl
// ---- World ----
#Floor Platform
{
    x : 400
    y : 580
    width : 800
    height : 40
    color : green
    sprite : "assets/dirt.jpg"
    kinematic : on          // solid, never moves
    collider : box
}

// ---- Player ----
#Player Character
{
    x : 100
    y : 500
    size : 32
    sprite : "assets/character.jpg"
    speed : 320
    jump_force : 500
    mass : 1
    gravity : on            // falls and lands on the floor
    friction : 0.4
    collider : box
}

// ---- Collectible ----
#Coin1 Item
{
    x : 400
    y : 540
    size : 16
    sprite : "assets/coin.png"
    gravity : off           // floats in place
    trigger : on            // overlap detected, no physical bump
    collider : box
}

// ---- Controls ----
on a     { move left  }
on d     { move right }
on space { jump       }

// ---- Rules ----
on collide Coin
{
    despawn                 // remove the coin
    play pickup             // and play a sound
}
```

What happens: the Lexer/Parser build one platform, one character, and one item, and register four handlers. On start, the Player falls (`gravity : on`) and lands on the Floor. Holding `D` runs `move right` each frame. Touching `Coin1` fires the `on collide Coin` handler because `Coin1` matches the target `Coin`.

### Recipes

**Collectible that heals**

```argdl
#Coin1 Item { x : 250  y : 380  size : 16  sprite : "assets/powerupBlue.png"
              gravity : off  trigger : on  collider : box }

on collide Coin { despawn  heal Player 2  play pickup }
```

**Patrolling enemy**

```argdl
#Enemy1 Enemy
{
    x : 400
    y : 540
    size : 32
    sprite : "assets/enemyBlack1.png"
    gravity : on
    health : 3
    patrol_min : 200      // walks back and forth between x = 200 ...
    patrol_max : 700      // ... and x = 700
    patrol_speed : 70
    collider : box
}
```

**Turret that aims and fires on a timer**

```argdl
#TurretLeft Item
{
    x : 400  y : 416  size : 40
    sprite : "assets/turretBase_big.png"
    mass : 999  gravity : off  kinematic : on
    health : 8  tag : Turret  collider : box
}

#EnemyBulletPrefab Prefab
{
    tag : EnemyBullet  size : 10  sprite : "assets/laserRed01.png"
    physics : off  collider : box
    projectile_speed : 300  projectile_lifetime : 3.0  projectile_damage : 1
}

on timer 4
{
    look_at TurretLeft Player           // aim
    TurretLeft.spawn EnemyBulletPrefab  // fire, from the turret itself
}
```

**Timed enemy spawner**

```argdl
#SpawnerLeft Item
{
    x : 150  y : 100  size : 24
    sprite : "assets/spaceStation_018.png"
    mass : 999  gravity : off  kinematic : on  health : 4
    tag : Spawner  skip_collision : on
    spawn_every : 8              // a new grunt every 8 seconds
    spawn_prefab : GruntPrefab
    spawn_at_self : on           // ... at the spawner's position
}
```

**Falling hazard (meteors)**

```argdl
#MeteorPrefab Prefab
{
    tag : Meteor  size : 28  sprite : "assets/meteorBig.png"
    physics : off  collider : box
    projectile_speed : 340  projectile_lifetime : 5.0  projectile_damage : 3
    projectile_angle : 90              // straight down (+y is down)
    projectile_friendly_tag : Enemy    // does not hurt enemies
}

on timer 5 { spawn MeteorPrefab at 300 -50 }   // starts above the screen
```

**Boss after a delay**

```argdl
#BossPrefab Prefab
{
    tag : Enemy  size : 56  sprite : "assets/enemyRed1.png"
    mass : 3  gravity : on  health : 15
    boss : on  hp : 15  collider : box
}

on timer 60 { spawn BossPrefab at 600 100 }    // repeats every 60 seconds
```

**Win condition: reach a portal**

```argdl
#ExitPortal Item
{
    x : 1900  y : 500  size : 40
    sprite : "assets/powerupBlue.png"
    gravity : off  trigger : on  tag : Portal  collider : box
}

on collide Portal { victory  play pickup }
```

**Delayed, one-time event**

```argdl
after 25 { spawn ExtraSpawnerPrefab at 500 100 }   // once, 25 s in
```

**Facing direction plus multiple key bindings**

```argdl
on a     { move left   rotate 180 }
on left  { move left   rotate 180 }
on d     { move right  rotate 0   }
on right { move right  rotate 0   }
```

### The Bundled Example Games

Three ARGDL levels ship with the engine. They deliberately use different subsets of the language so you can learn from each.

| Level | Theme | Win / lose | ARGDL features it demonstrates |
|---|---|---|---|
| **Platformer** (`game.argdl`) | Mario-style side scroller, 3000 units wide | Reach the flagpole; falling below the platforms is game over | Wide parallax `Background`, many `Platform` entities (ground, pipes, floating platforms, stairs, flagpole), 12 trigger `Item` coins, 5 `Enemy` entities, physics crates (`mass`, `restitution`), keyboard-only input handlers |
| **Turret Arena** | Single-screen space arena | Survive: defeat enemies while turrets fire and a boss arrives at 60 s | `health`, `invuln_duration`, `knockback`, `Prefab` bullets, `on j { spawn ... }` shooting, turrets aimed with `look_at` and fired on `on timer`, `spawn_every` spawners, `patrol_*` enemies, `on collide` damage and healing, timed boss spawn |
| **Meteor Run** | Wider space level with a goal | Reach the `ExitPortal` to win; health is limited to 6 | Everything above, plus falling meteor prefabs (`projectile_angle`, `projectile_friendly_tag`), several concurrent `on timer` handlers, `after` one-shot spawns, an `on collide Portal { victory }` win condition |

Suggested reading order: the platformer (pure layout and physics), then the Turret Arena (combat and timers), then the Meteor Run (a complete objective-driven level).

### Tips and Gotchas

- **Names must be unique.** Handlers such as `TurretLeft.spawn` and `look_at TurretLeft Player` rely on exact entity names.
- **Repeated things share a name prefix**: `Coin1`..`Coin12`, so one `on collide Coin` handler covers them all.
- **Use `trigger : on` for pickups and goals** so the player passes through them instead of bumping into them.
- **Use `kinematic : on` for anything solid that should stay put**: platforms, turrets, spawners.
- **Turrets and other immovable objects** work best with a very large `mass` (for example `999`) in addition to `kinematic : on`.
- **Set `physics : off` and `collider : box` on projectiles** so they fly straight instead of falling or being pushed.
- **Remember +y is down.** A projectile angle of `90` is straight down, and spawning at a negative `y` places something above the visible screen.
- **Sprite paths are relative to the working directory**, so keep the `assets/` folder next to the executable.
- **Every entity that should collide needs `collider : box`.** Entities with `skip_collision : on` are ignored by collision entirely.
- **Timers repeat, `after` fires once.** Use `after` for one-time events and `on timer` for repeating ones.

---

## Technical Details

### Physics System
- Rigid body dynamics with impulse-based collision resolution
- AABB (Axis-Aligned Bounding Box) collision detection
- Ground detection and friction
- Support for static, kinematic, and dynamic bodies
- Layer-based collision filtering
- Trigger volumes that report overlaps without physical response

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
- Lexer, Parser, Interpreter, and RuntimeBridge pipeline for the scripting language

## Controls

The keyboard controls are defined in the ARGDL script (`on <key>` handlers), so they can be changed without touching C++. The bundled levels use:

- **A / Left Arrow**: Move left
- **D / Right Arrow**: Move right
- **W / Up Arrow / Space**: Jump
- **J**: Shoot (combat levels)
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
- Reach the flagpole (platformer) or the exit portal (Meteor Run) to complete the level
- Falling below platforms results in game over
- In combat levels, the player has limited health, brief invulnerability after each hit, and knockback when hurt
- Camera follows the player smoothly

## License

This project is licensed under the [MIT License](LICENSE).

## Credits

Built with C++, OpenGL, SDL2, and custom game engine architecture.
