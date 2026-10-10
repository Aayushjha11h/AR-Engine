<img width="1439" height="758" alt="Screenshot 2026-09-19 170227" src="https://github.com/user-attachments/assets/b785d407-826c-4ebd-b2ca-b61df55d4289" />
<img width="1003" height="775" alt="Screenshot 2026-09-19 170157" src="https://github.com/user-attachments/assets/555b22e7-fa41-4685-b74a-c46ec03931ea" />
<img width="1004" height="789" alt="Screenshot 2026-09-19 170137" src="https://github.com/user-attachments/assets/29ce6b08-fc85-4f0c-8f9a-83a9d1811c21" />
<img width="992" height="764" alt="Screenshot 2026-09-19 163744" src="https://github.com/user-attachments/assets/e0f8ccad-a1a1-4496-84ce-8c9d7ddaeba6" />

# AR-Engine

A custom 2D game engine built from scratch with C++ and OpenGL, featuring its own scripting language, **ARGDL** (AR Game Definition Language), for describing levels, entities, and game logic.

> **Note:** "AR" is not Augmented Reality. It is the initials of the my name.

You describe a game in a text file, not in C++. Entities, physics colliders, and event logic are all declarative, and a small playable scene fits in under 100 lines. Edit the file, restart, and the level changes. **No recompile.**

The core idea: anyone without much knowledge of programming languages or game engines should be able to make a playable game within 30 minutes to an hour.

The engine includes a complete physics system, an OpenGL rendering pipeline, combat and AI systems, and demo games, including a Mario-style platformer and a wave-survival shooter. With the latest language update, ARGDL is no longer just a list of entities and commands. It now has **blueprints with inheritance, variables, expressions, conditions, loops, event guards, and world queries**, while keeping the readable, declarative style it started with.

## Table of Contents

- [Features](#features)
- [What's New (ARGDL v2)](#whats-new-argdl-v2)
- [Project Structure](#project-structure)
- [Building](#building)
- [Running the Game](#running-the-game)
- [Engine Architecture](#engine-architecture)
  - [Entity-Component System](#entity-component-system)
  - [Components](#components)
  - [Physics](#physics)
  - [Rendering](#rendering)
  - [Camera, Input, Audio](#camera-input-audio)
  - [Game Loop and Game States](#game-loop-and-game-states)
- [ARGDL Scripting Language](#argdl-scripting-language)
  - [Design Goals](#design-goals)
  - [How a Script Becomes a Game](#how-a-script-becomes-a-game)
  - [Language Overview](#language-overview)
  - [Lexical Rules](#lexical-rules)
  - [Formal Grammar](#formal-grammar)
  - [1. Objects](#1-objects)
  - [2. Blueprints and Inheritance](#2-blueprints-and-inheritance)
  - [3. Instances](#3-instances)
  - [4. Property Reference](#4-property-reference)
  - [5. Events and Triggers](#5-events-and-triggers)
  - [6. Event Guards](#6-event-guards)
  - [7. Variables](#7-variables)
  - [8. Expressions](#8-expressions)
  - [9. Member Access](#9-member-access-reading-other-entities)
  - [10. Built-in Functions](#10-built-in-functions)
  - [11. Control Flow](#11-control-flow)
  - [12. Command Reference](#12-command-reference)
  - [13. Targets and Matching](#13-targets-and-matching)
  - [14. Coordinate System](#14-coordinate-system)
  - [Walkthrough: A Minimal Level](#walkthrough-a-minimal-level)
  - [Recipes](#recipes)
  - [The Bundled Example Games](#the-bundled-example-games)
  - [Migrating from ARGDL v1](#migrating-from-argdl-v1)
  - [Tips and Gotchas](#tips-and-gotchas)
  - [Current Limitations](#current-limitations)
- [Technical Details](#technical-details)
- [Controls](#controls)
- [Game Mechanics](#game-mechanics)
- [Roadmap](#roadmap)
- [License](#license)
- [Credits](#credits)

## Features

### Core Engine Systems
- **Physics System**: Rigid body dynamics with gravity, AABB collision detection, impulse resolution, friction, restitution, linear drag, and trigger volumes
- **Rendering**: OpenGL 3.3 sprite rendering with texture loading, render layers, and parallax backgrounds
- **Entity-Component Architecture**: Flexible component system for game objects
- **Custom Scripting Language (ARGDL)**: Human-readable game definition language for level design, input bindings, collision rules, timers, spawning, and now full game logic (state, conditions, loops)
- **Input Handling**: Keyboard and mouse input processing, with multiple bindings per action
- **Camera System**: Follow camera with smooth interpolation and zoom support
- **Audio System**: Sound playback with SDL2
- **Game State Management**: Title screen, playing, game over, and victory states
- **Combat and AI**: Health, damage, invulnerability frames, knockback, projectiles, patrolling enemies, chasing enemies, bosses, and timed spawners, all configurable from ARGDL

### ARGDL Language Features
- **Objects, Blueprints, and Instances**: reusable templates with multi-level inheritance, one-line placement
- **Variables**: remember scores, combos, waves, and timers across events
- **Expressions**: arithmetic, comparison, logic, parentheses, operator precedence, type coercion
- **Control flow**: `if` / `else if` / `else`, `for ... in range`, `while`
- **Event guards**: an event only fires when its `if` condition is true
- **World queries**: read another entity's position, health, or variables; `distance_to`, `exists`, `random`
- **Smarter movement**: `move_toward` for chasing enemies

### Demo Game Features
The included demos show what the engine can do:

- **Platformer**: a Mario-style level with physics-based movement, 12 collectible coins, 5 enemies, ground platforms, walls, pipes, floating platforms, stairs, interactive crates with mass and restitution, a flagpole victory condition, and death zones
- **Combat levels (Turret Arena and Meteor Run)**: shooting, turrets, patrolling enemies, spawners, falling meteors, a timed boss, and a portal victory condition
- **Wave Survival (new)**: a full v2 showcase using blueprint inheritance, variables, waves that escalate over time, a shooting combo system, chasing enemies, turrets, meteors, a delayed boss, and a portal that only works after you have survived long enough

See [The Bundled Example Games](#the-bundled-example-games).

### What You Can Build
2D platformers, shooters, and arcade games such as:

- Doodle Jump / Abacus Jump
- Single-Screen Mario
- Space Invaders / Galaga
- Flappy Bird / Jetpack Joyride
- Bullet Hell / Hazard Dodge
- Single-Screen Boss Battle
- Pac-Man / Frogger

## What's New (ARGDL v2)

ARGDL went from a flat entity-and-command format to a full scripting language with state, control flow, cross-entity queries, and event guards, while keeping the declarative object/blueprint/instance model that made it readable in the first place.

Everything below is covered in detail in the [language reference](#argdl-scripting-language).

| Area | Before (v1) | Now (v2) |
|---|---|---|
| **Templates** | `Prefab` blocks, no formal relationship between them; each repeated the properties it needed | `Blueprint` blocks with `extends`. Multi-level inheritance, forward references, and cycle detection at load time |
| **Placing entities** | Every entity was a full `#Name Type { ... }` block | One-line instances: `#Enemy1 Grunt at 500 540`, with optional per-instance overrides |
| **Spawning** | `spawn <Prefab>` | `spawn <Blueprint>` through the same registry as declarative instances. There is no separate prefab list |
| **Events** | Flat command lists that always ran every command in order | Full blocks with local logic: they can read variables, branch, loop, and query the world |
| **Event conditions** | None | `if` guards: `on collide Coin if $coins < 4 { ... }` |
| **Variables** | None, no memory across events | Per-entity variables: declare as properties, change with `set`, read with `$name` |
| **Expressions** | Only literal values | `+ - * / %`, comparisons, `and` / `or` / `not`, parentheses, type coercion |
| **Reading other entities** | Impossible | `Player.health`, `Boss.x`, `Player.coins`, and more |
| **Built-in functions** | None | `distance_to`, `random`, `exists` |
| **Branching** | None | `if` / `else if` / `else`, arbitrarily nested |
| **Loops** | None | `for i in range a b` and `while` (with a safety cap) |
| **Chasing** | Directional moves only | `move_toward <Entity>` |
| **New keywords** | `on`, `after`, `collide`, `timer` | Adds `set`, `if`, `else`, `while`, `for`, `in`, `range`, `and`, `or`, `not`, `true`, `false`, `nil`, `exists`, `extends`, `at` |
| **New operators** | `:` | Adds `$`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `+`, `-`, `*`, `/`, `%`, `(`, `)` |

### v1 versus v2 at a glance

**Before**: two identical enemies meant two copy-pasted blocks. Changing enemy health meant editing both.

```argdl
#Enemy1 Enemy {
    x : 400
    y : 540
    size : 32
    speed : 60
    health : 2
}

#Enemy2 Enemy {
    x : 800
    y : 540
    size : 32
    speed : 60
    health : 2
}
```

**Now**: define the shared properties once, then place instances in one line.

```argdl
#Grunt Blueprint {
    size : 32
    speed : 60
    health : 2
}

#Enemy1 Grunt at 400 540
#Enemy2 Grunt at 800 540
```

**Before**: a coin always did the same thing.

```argdl
on collide Coin {
    despawn
    heal Player 2
    play pickup
}
```

**Now**: a coin can count itself, roll for a bonus, and stop working once you have enough.

```argdl
on collide Coin if $coins < 4 {
    despawn
    set coins = $coins + 1
    heal Player 2
    play pickup

    set lucky = random 1 3
    if $lucky > 2 {
        heal Player 2
    }
}
```

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
│   ├── Parser.cpp/h       # AST parser: objects, blueprints, instances, events, statements, expressions
│   ├── Interpreter.cpp/h  # Blueprint resolution, instance creation, event registration
│   ├── Value.cpp/h        # Dynamic value type (int, float, bool, string, nil) and coercion rules
│   ├── Expr.cpp/h         # Expression evaluation (literals, variables, members, calls, operators)
│   ├── Statement.h        # Statement node definitions (command, set, if/else, while, for) used in event bodies (header-only)
│   ├── RuntimeBridge.cpp/h # Bridge connecting ARGDL to C++ engine core; statement execution and commands
│   └── Token.cpp/h        # Lexical token definitions
├── ThirdParty/            # External dependencies
│   └── stb/               # stb_image for image texture decoding
├── assets/                # Game graphics assets
├── game.argdl             # Wave Survival showcase level (ARGDL v2)
├── game2.argdl            # Demo game script 2
├── game3.argdl            # Shooter platformer demo game script 3
├── main.cpp               # Application entry point
└── README.md              # Project documentation
```

`Sprite.h` and `Statement.h` are header-only (no matching `.cpp`). Every other file listed above has both a `.h` and a `.cpp`.

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
6. Press **J** to shoot in the combat levels
7. Collect coins and reach the goal to win
8. Press **R** to restart after game over

The game loads `game.argdl` from the working directory. To play a different level, point the loader at another `.argdl` file. To change a level, edit the file and restart. **No recompile is needed.**

---

## Engine Architecture

AR-Engine is a **data-driven, component-based 2D engine**. Engine code (C++) handles the low-level systems, and game logic lives in the scripting language (ARGDL).

Key design principles:

1. **Separation of concerns**: the engine handles rendering, physics, audio, and input. The language handles game content and rules.
2. **Components, not inheritance hierarchies**: entities are containers of components.
3. **Runtime interpretation**: scripts are parsed and executed at load time, so iteration is fast.
4. **Bridge pattern**: `RuntimeBridge` is the only layer that touches the engine, so the language and the engine can evolve independently.

### Entity-Component System

An **Entity** has a unique name, a tag used for categorization (`Player`, `Enemy`, `Coin`, ...), an active flag, a dictionary of **variables** (this is what ARGDL's `$name` reads and `set` writes), and a set of components.

```cpp
class Entity {
    std::string Name;
    std::string Tag;
    bool Active = true;
    std::unordered_map<std::string, Value> Variables;   // per-entity script state
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_Components;
    std::vector<Component*> m_ComponentList;            // ordered updates
};
```

Components share a simple base interface (`OnAttach`, `Update(dt)`, `Render(renderer)`). They talk to each other through their shared owner entity, and all coordination between systems goes through scripts or the bridge.

### Components

| Component | Purpose | Key Properties |
|---|---|---|
| **Transform** | Position, rotation, scale | `Position`, `Rotation`, `Scale` |
| **SpriteRenderer** | Visual representation | `Texture`, `Color`, `Size`, `Layer`, `SkipCameraParallax` |
| **RigidBody** | Physics simulation | `Velocity`, `Mass`, `UseGravity`, `Friction`, `Restitution`, `LinearDrag`, `IsKinematic` |
| **Collider** | Collision detection | `Size`, `Layer`, `Mask`, `IsTrigger`, `OnCollision`, `OnTriggerEnter` |
| **Health** | Hit points and damage | `Current`, `Max`, `InvulnDuration`, `KnockbackForce` |
| **PatrolAI** | Patrolling and chasing behavior | `MinX`, `MaxX`, `Speed`, `Chasing`, `ChaseTimeout` |
| **BossAI** | Boss enemy behavior | `Health`, `MaxHealth`, `PlayerTarget` |
| **Spawner** | Timed entity creation | `PrefabName`, `Interval`, `InitialDelay`, `MaxSpawns`, `SpawnAtSelf` |
| **Projectile** | Bullet and meteor behavior | `Speed`, `Lifetime`, `Damage`, `AngleDeg`, `Shooter`, `FriendlyTag` |

The `RuntimeBridge` attaches components automatically based on which properties an entity or blueprint declares. For example, `health` adds a Health component, `patrol_min` / `patrol_max` add PatrolAI, `boss : on` adds BossAI, `spawn_every` adds a Spawner, and `projectile_speed` adds a Projectile.

### Physics

The physics system is custom, with no external physics library.

- Gravity with a configurable scale
- Velocity-based movement
- AABB (axis-aligned bounding box) collision detection with separation-based resolution
- Impulse-based collision response
- Friction, restitution (bounciness from 0 to 1), and linear drag (air resistance)
- Kinematic bodies: solid, but moved only by code, never by forces
- Trigger colliders: detect overlap and fire events with no physical response
- Collision layers and masks for filtering

### Rendering

- OpenGL 3.3 Core Profile with custom shaders
- Batched quad rendering for sprites
- Texture loading and caching with `stb_image`
- Parallax backgrounds (scrolling at a fraction of camera speed for depth)
- Render layers, drawn in this order: `background`, `scenery`, `main`, `foreground`
- UI overlays: health bar (green above 50%, orange above 25%, red below), coin counter, and title / game over / victory banners with pulsing text

Frame order: clear, parallax background, entities by layer, UI overlays, present.

### Camera, Input, Audio

- **Camera**: follows the player with smooth interpolation and supports zoom (mouse scroll)
- **Input**: key-down and key-pressed tracking, scancode-based so it is keyboard-layout independent. Several keys can be bound to the same action (`on a` and `on left`)
- **Audio**: SDL2 sound effects loaded and cached by name. `play pickup` plays `sounds/pickup.wav`

### Game Loop and Game States

Each frame: handle SDL events, compute delta time, update the script event system (when playing), update physics, update entities, render, render UI, swap buffers.

| State | Behavior |
|---|---|
| **Title Screen** | Shows the title and waits for Enter or Space |
| **Playing** | Runs the game and processes script events |
| **Game Over** | Shown when the player dies. Press R to restart |
| **Victory** | Shown when a script runs `victory` |

Restarting reloads the script, so every level starts from a clean state.

---

## ARGDL Scripting Language

ARGDL (**AR Game Definition Language**) is a small, declarative domain-specific language (DSL) that describes an entire game: what exists in the world, how it looks and behaves physically, and what happens when the player presses keys, things collide, or time passes.

Instead of recompiling C++ every time you move a platform or change enemy health, you edit a plain-text `.argdl` file and re-run the game.

A script contains three kinds of top-level construct, and everything else is a detail inside them:

| Construct | Starts with | Purpose |
|---|---|---|
| **Object** | `#Name Type { ... }` | Declares a unique entity placed in the world (the player, a backdrop) |
| **Blueprint** | `#Name Blueprint { ... }` | A reusable template, optionally inheriting from another with `extends` |
| **Instance** | `#Name Blueprint at x y` | Places a copy of a blueprint in the world, with optional overrides |
| **Event handler** | `on` or `after` | Declares *what happens when* a key is pressed, two things collide, or a timer fires, optionally gated by an `if` guard |

Entities are lists of `property : value` pairs. Handlers are blocks of statements that can use variables, conditions, and loops.

### Design Goals

- **Declarative first**: you say *what* exists and *what should happen*, not how to allocate or render.
- **Readable**: a level file reads like a description of the level.
- **Event-driven**: react to keys, collisions, and time.
- **Minimal syntax**: no semicolons, no type declarations, no boilerplate.
- **Engine-native**: every property maps directly onto an engine component.
- **Iterable**: change a number, restart, see the result.
- **Reusable**: blueprints mean you never write the same enemy twice.

### How a Script Becomes a Game

```
 game.argdl  ─►  Lexer  ─►  Parser  ─►  Interpreter  ─►  RuntimeBridge  ─►  Engine
 (text)         (tokens)    (syntax     (resolves         (creates entities,   (physics,
                             tree)       blueprints,       runs statements,     rendering,
                                         builds events)    dispatches           audio, ...)
                                                           commands)
```

1. **Lexer** (`Language/Lexer.cpp`) turns raw text into tokens (`#`, names, `{`, `}`, `:`, numbers, strings, keywords, operators, `$`).
2. **Parser** (`Language/Parser.cpp`) checks the tokens against the grammar and builds a syntax tree of objects, blueprints, instances, and events. Event bodies contain statements, and statements contain expressions.
3. **Interpreter** (`Language/Interpreter.cpp`) resolves blueprint inheritance (with cycle detection and caching), creates instance definitions, and registers event handlers so they can run later.
4. **RuntimeBridge** (`Language/RuntimeBridge.cpp`) is the only layer that touches the engine. It turns script objects into entities and components, executes statements, evaluates guards, and dispatches commands into engine calls.

Keeping the bridge separate means the language can evolve without the engine depending on script syntax, and vice versa.

### Language Overview

A tiny script showing the major pieces:

```argdl
#Coin Blueprint {                 // <- blueprint: a reusable template
    size : 16
    gravity : off
    trigger : on
    collider : box
    tag : Coin
}

#Player Character {               // <- object: one unique entity
    x : 100
    y : 500
    size : 32
    gravity : on
    collider : box
    coins : 0                     // <- a custom property becomes a variable
}

#Coin1 Coin at 400 540            // <- instance: one line

on d {                            // <- event handler
    move right                    // <- command
}

on collide Coin if $coins < 5 {   // <- guard: only fires while coins < 5
    despawn
    set coins = $coins + 1        // <- statement: set a variable
    if $coins == 5 {              // <- statement: branch
        victory
    }
}
```

### Lexical Rules

- **Whitespace and line breaks** are not significant; one property or statement per line is the convention.
- **Numbers** may be integers or decimals (`320`, `0.5`, `1.5`).
- **Booleans** in properties are the words `on` and `off`. In expressions, `true` and `false` are also available, along with `nil`.
- **Strings** are double-quoted and used for file paths and text values: `"assets/coin.png"`.
- **Bare words** are used for colors (`red`, `cyan`), collider shapes (`box`), tags (`Turret`), names, and blueprint names.
- **Names are case-sensitive.** `Player` and `player` are different names.
- **Comments** with `//` are used throughout the examples in this README.
- **Special characters**: `#` starts an entity, `$` reads a variable, `:` separates a property from its value.

**Keywords**: `on`, `after`, `collide`, `timer`, `extends`, `at`, `set`, `if`, `else`, `while`, `for`, `in`, `range`, `and`, `or`, `not`, `true`, `false`, `nil`, `exists`, `distance_to`, `random`.

### Formal Grammar

An informal EBNF-style description of the language:

```
program      = { object | blueprint | instance | event } ;

object       = "#" Name Type "{" { property } "}" ;
blueprint    = "#" Name "Blueprint" [ "extends" Name ] "{" { property } "}" ;
instance     = "#" Name Blueprint "at" number number [ "{" { property } "}" ] ;
property     = key ":" value ;
value        = number | "on" | "off" | word | string ;

event        = "on" trigger [ "if" expr ] block
             | "after" number [ "if" expr ] block ;

trigger      = key                       (* a, d, space, left, j, ...   *)
             | "collide" Target          (* on collide Coin             *)
             | "timer" number ;          (* on timer 5                  *)

block        = "{" { statement } "}" ;

statement    = "set" name "=" expr
             | "if" expr block { "else" "if" expr block } [ "else" block ]
             | "while" expr block
             | "for" name "in" "range" expr expr block
             | command ;

command      = "move" ("left" | "right" | "up" | "down")
             | "move" "$"name
             | "jump"
             | "rotate" number
             | "gravity" ("on" | "off")
             | "play" word
             | "spawn" Name [ "at" expr expr ]
             | Name "." "spawn" Name
             | Name "." "move_toward" Target
             | "despawn"
             | "heal" Target number
             | "damage" [ Target ] number
             | "look_at" Name Target
             | "apply_force" number number
             | "victory" ;

expr         = or_expr ;
or_expr      = and_expr { "or" and_expr } ;
and_expr     = equality { "and" equality } ;
equality     = relational { ("==" | "!=") relational } ;
relational   = additive { ("<" | ">" | "<=" | ">=") additive } ;
additive     = term { ("+" | "-") term } ;
term         = unary { ("*" | "/" | "%") unary } ;
unary        = ("-" | "not") unary | primary ;
primary      = number | string | "true" | "false" | "nil"
             | "$" name
             | Name "." member
             | "distance_to" Target
             | "random" expr expr
             | "exists" Target
             | "(" expr ")" ;
```

The parser tells statements and commands apart by the first token: `set`, `if`, `while`, and `for` start statements, and anything else is a command.

### 1. Objects

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

Objects are the escape hatch for "this thing is unique, don't template it". The player is the classic example:

```argdl
#Player Character {
    x : 200
    y : 480
    size : 36
    speed : 400
    jump_force : 900
    health : 10
    coins : 0
    score : 0
}
```

#### Entity types

| Type | Used for | Notes |
|---|---|---|
| `Character` | The player-controlled character | Typically has `speed`, `jump_force`, `mass`, and (in combat levels) `health`, `invuln_duration`, `knockback` |
| `Platform` | Ground, walls, pipes, stairs, floating platforms | Usually `kinematic : on` so it is solid but never moves. Sized with `width` and `height` |
| `Item` | Coins, crates, blocks, turrets, spawners, portals | A general-purpose "thing"; its behavior comes from its properties (`trigger`, `kinematic`, `tag`, `spawn_every`, ...) |
| `Enemy` | Enemies | Add `patrol_min`, `patrol_max`, `patrol_speed` for patrol AI, or `boss : on` for boss AI |
| `Background` | Parallax background layer | Usually `skip_collision : on` and a small `parallax` factor |
| `Scenery` | Decorative elements (for example trees) | Purely visual |
| `Blueprint` | A **template** that is not placed in the world until instanced or spawned | Has no `x` / `y`; see the next section |

### 2. Blueprints and Inheritance

A **Blueprint** is a reusable entity template. It is declared with normal block syntax but is **not placed in the world**. It waits until an instance line or a `spawn` command creates a copy.

Blueprints replace v1 `Prefab` blocks and add **inheritance** with `extends`:

```argdl
#Enemy Blueprint {
    size : 32
    color : purple
    health : 3
    speed : 160
    patrol_speed : 80
    tag : Enemy
}

#Grunt Blueprint extends Enemy {
    health : 2
    patrol_min : 100
    patrol_max : 1900
}

#Brute Blueprint extends Enemy {
    size : 48
    color : red
    health : 8
    mass : 3
    patrol_speed : 45
    speed : 100
}

#Hunter Blueprint extends Enemy {
    size : 36
    color : magenta
    health : 4
    boss : on
    hp : 4
}
```

Three kinds of enemy, and each one writes only its **differences** from `Enemy`. The shared properties live once, in the base.

Rules:

- **Multi-level chains** are allowed and can be arbitrarily deep.
- **Child properties override parent properties.**
- **Forward references work**: a child can be declared before its parent.
- **Cycles are detected** (A extends B extends A) and rejected at load time with an error.
- Resolved blueprints are **cached**, so inheritance is computed once.

Deeper chains are useful for variants:

```argdl
#Bullet Blueprint {
    color : cyan
    sprite : "assets/laserBlue01.png"
    physics : off
    collider : box
}

#PlayerBullet Blueprint extends Bullet {
    tag : PlayerBullet
    size : 12
    projectile_speed : 1100
    projectile_lifetime : 0.7
    projectile_damage : 1
}

#HeavyBullet Blueprint extends PlayerBullet {
    size : 20
    color : white
    projectile_damage : 3
    projectile_speed : 900
}
```

`HeavyBullet` gets everything from `Bullet` and `PlayerBullet` and overrides only what it needs. Change the base sprite and all three variants update. Change `HeavyBullet`'s color and only it changes.

Blueprints are how you build anything that appears at runtime: bullets, meteors, wave enemies, bosses, and even spawners.

### 3. Instances

An instance places a copy of a blueprint into the world.

**Declarative instance**: top level, created when the level loads.

```argdl
#Enemy1 Grunt at 500 540
#Enemy2 Grunt at 1000 540
#Enemy3 Brute at 1500 540
```

One line each: name, blueprint, position.

**Instance with overrides**: the blueprint supplies defaults and the instance changes only the difference.

```argdl
#ExitPortal Portal at 1600 500

#WallLeft Platform at 30 300 {
    width : 40
    height : 600
}

#BossVariant Boss at 1000 100 {
    hp : 50
    size : 100
}
```

**Dynamic instance**: created at runtime from an event with `spawn`.

```argdl
on timer 30 {
    spawn Boss at 1000 -80
}

on space {
    spawn PlayerBullet
}

on collide Coin if $coins >= 5 {
    for i in range 1 2 {
        spawn Grunt at 400 100
    }
}
```

`spawn <BlueprintName>` looks up the blueprint, builds the entity, and places it. The name resolves through the same registry as declarative instances, so there is no separate prefab list. The spawn position can also be computed:

```argdl
set ex = random 300 1700
spawn Hunter at $ex 100
```

### 4. Property Reference

#### Position and size

| Property | Value | Meaning |
|---|---|---|
| `x`, `y` | number | Position of the entity's **center**, in world units |
| `width`, `height` | number | Dimensions, used for platforms and backgrounds |
| `size` | number | Uniform size for square-ish things (characters, items, enemies, projectiles) |
| `rotation` | degrees | Initial rotation |

#### Appearance

| Property | Value | Meaning |
|---|---|---|
| `color` | color name | Tint / fallback color (`red`, `green`, `blue`, `cyan`, `yellow`, `purple`, `gray`, `brown`, `orange`, `magenta`, `white`, ...) |
| `sprite` | `"path"` | Texture file, relative to the working directory (for example `"assets/coin.png"`) |
| `layer` | `background`, `scenery`, `main`, `foreground` | Render order layer |
| `parallax` | number | Background scroll factor. `0.1` scrolls at 10% of camera speed, giving depth |

#### Movement and physics

| Property | Value | Meaning |
|---|---|---|
| `speed` | number | Horizontal movement speed used by `move` and `move_toward` |
| `jump_force` | number | Impulse applied by `jump` |
| `mass` | number | Physics mass. Large values (for example `999`) make an object effectively immovable in collisions |
| `gravity` | `on` / `off` | Whether gravity affects this entity |
| `gravity_scale` | number | Multiplier on gravity |
| `friction` | number | Surface friction coefficient |
| `restitution` | 0 to 1 | Bounciness |
| `linear_drag` | number | Air resistance on horizontal velocity |
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

#### Projectiles (for blueprints that get spawned as bullets or hazards)

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
| `spawn_prefab` | Blueprint name | Which blueprint to spawn. (The property keeps its original name even though templates are now called blueprints) |
| `spawn_at_self` | `on` / `off` | Spawn at the spawner's own position |

#### Custom variables

Any other property you declare, such as `coins : 0`, `score : 0`, `wave : 1`, becomes a **variable** on that entity. See [Variables](#7-variables).

### 5. Events and Triggers

Handlers bind a **trigger** to a **block of statements**.

```
on <trigger> { statements }
on <trigger> if <condition> { statements }

after <seconds> { statements }
after <seconds> if <condition> { statements }
```

| Trigger | Example | Fires when |
|---|---|---|
| Key name | `on a`, `on space`, `on left`, `on j` | The key is pressed / held. Letter keys, `space`, and the arrow keys (`left`, `right`, `up`, `down`) are used in the bundled scripts |
| Collision | `on collide Coin` | The player collides with something matching the target (a **new** contact, so a pickup fires once, not every frame) |
| Repeating timer | `on timer 5` | Every 5 seconds, repeatedly. Fractions work (`on timer 0.5`) |
| One-shot delay | `after 25` | Once, 25 seconds after the level starts |

Notes:

- Several handlers can bind the same key (`on a` and `on left` both move left), so you can offer multiple control schemes.
- Several timers can share an interval or coexist. The Wave Survival level runs a dozen at once, each independent.
- Collision handlers only fire for contacts involving the player, so enemy-versus-bullet contacts do not trigger `on collide` rules.
- Statements in a handler run top to bottom.
- Jump-style actions respond to a fresh key press, not a held key, so holding the key does not cause continuous jumping.

### 6. Event Guards

Any event can carry an `if` guard between the trigger and the body. If the guard is false, the event does not fire.

```argdl
on collide Coin if $coins < 4 { ... }
on collide Coin if $coins >= 4 { ... }

on timer 30 if $coins >= 3 { spawn Boss }
on timer 60 if $coins < 3  { spawn Boss }

on collide Portal if $survived >= 60 { victory }
on collide Portal if $survived < 60  { play hurt }

after 25 if $coins >= 2 { spawn Spawner at 500 100 }
```

This is what makes events react to **state** instead of only reacting to input. Two events with the same trigger can now do completely different things depending on the game's status. In the Wave Survival level, the portal refuses to end the game until you have survived 60 seconds, and the waves of enemies are gated with guards like `on timer 9 if $wave >= 4`.

### 7. Variables

Every entity has a dictionary of variables. Declare them as properties, change them with `set`, read them with `$`.

**Declare** them as properties on an object or blueprint:

```argdl
#Player Character {
    coins : 0
    score : 0
    wave : 1
    survived : 0
    combo : 0
}
```

**Set** a new value:

```argdl
set coins = $coins + 1
set score = 0
set alert = "medium"
set alive = true
```

**Read** with `$name` in expressions:

```argdl
if $coins >= 5 { ... }
if $survived == 60 { ... }
while $sparks > 0 { ... }
```

**Substitute** inside commands. `$name` is replaced with the variable's string form:

```argdl
set dir = "left"
move $dir            // becomes: move left
```

Variable values can be integers, floats, booleans, strings, or `nil`. Reading a variable that does not exist gives `nil`.

In the bundled levels, all game-wide state (coins, score, wave, survival time, combo) is stored on the `Player`.

### 8. Expressions

Expressions have a full operator set, type coercion, and parentheses.

**Arithmetic**

```argdl
set total = 3 + 4 * 2      // 11
set half  = 10 / 3         // 3   (integer division when both sides are integers)
set rest  = 10 % 3         // 1
set neg   = 0 - 5          // -5
```

**Comparison**

```argdl
if health > 5 { ... }
if $coins == 3 { ... }
if $survived != 60 { ... }
if $wave <= 2 { ... }
```

**Logic**

```argdl
if $coins >= 3 and $wave >= 2 { ... }
if Player.health < 3 or $combo > 10 { ... }
if not $alive { ... }
```

**Precedence**, from highest to lowest:

| Level | Operators |
|---|---|
| 1 (highest) | unary `-`, `not` |
| 2 | `*`, `/`, `%` |
| 3 | `+`, `-` |
| 4 | `<`, `>`, `<=`, `>=` |
| 5 | `==`, `!=` |
| 6 | `and` |
| 7 (lowest) | `or` |

Parentheses override precedence:

```argdl
if ($coins > 3 and Player.health > 5) or $boss_dead { ... }
```

**Type coercion**

| Operation | Operands | Result | Example |
|---|---|---|---|
| `+` | string with anything | string concatenation | `5 + "3"` gives `"53"` |
| `+ - * /` | float with int | float | `1.5 * 2` gives `3.0` |
| `+ - * /` | int with int | int | `6 / 2` gives `3` |
| `==` | string with string | string equality | `"a" == "a"` is `true` |
| `==` | number with number | numeric equality (with tolerance) | `5 == 5.0` is `true` |
| `< > <= >=` | string with string | lexicographic | `"a" < "b"` is `true` |
| `< > <= >=` | number with number | numeric | `3 < 5` is `true` |

**Truthiness**: `nil`, `0`, `false`, `"false"`, and `"0"` are false. Everything else is true.

### 9. Member Access: Reading Other Entities

Before v2, events could affect entities through commands but could not look at them. Now any expression can read another entity's state:

| Form | Reads |
|---|---|
| `Player.x`, `Enemy1.y` | Position |
| `Player.health`, `Boss.health` | Current health |
| `Player.max_health` | Maximum health |
| `Player.vx`, `Player.vy` | Velocity |
| `Boss.phase`, `Player.coins` | Any custom variable |

```argdl
if Player.x > 1000 {
    spawn Grunt at 300 100
}

if Enemy1.health <= 1 {
    play pickup
}

if Boss.x < Player.x {
    set direction = "right"
}
```

### 10. Built-in Functions

Three built-ins, each callable with or without parentheses.

**`distance_to <Entity>`**: Euclidean distance from the current entity to the target.

```argdl
if distance_to Player < 400 {
    Enemy1.move_toward Player
}

if distance_to TurretLeft < 800 {
    TurretLeft.spawn EnemyBullet
}
```

**`random <lo> <hi>`**: an inclusive random integer.

```argdl
set lucky = random 1 3
set spawn_x = random 300 1700
spawn Hunter at $spawn_x 100
```

**`exists <Entity>`**: true if the entity currently exists in the scene.

```argdl
if exists Boss {
    if distance_to Boss < 900 {
        Boss.move_toward Player
    }
}
```

`exists` solves a specific problem: as entities get killed, timers that reference them keep firing. Without it, commands like `Boss.move_toward` would warn that the target is missing. With it, the guard skips them silently.

### 11. Control Flow

#### `if` / `else if` / `else`

```argdl
if $coins >= 5 {
    victory
}

if $lucky > 2 {
    heal Player 2
} else {
    play hurt
}

if Player.health < 4 {
    play hurt
} else if Player.health < 7 {
    set alert = "medium"
} else {
    set alert = "safe"
}
```

`if` statements nest to any depth:

```argdl
if exists Boss {
    if distance_to Boss < 900 {
        Boss.move_toward Player
    }
}
```

#### `for`

```
for <var> in range <start> <end> { ... }
```

An **inclusive** range. It counts up if `start <= end` and down otherwise.

```argdl
for i in range 1 3 {
    heal Player 1
}

for i in range 0 $max {
    spawn Grunt at 400 100
}
```

#### `while`

```argdl
set sparks = 5
while $sparks > 0 {
    set sparks = $sparks - 1
    play pickup
}
```

Loops run while the condition is truthy, with a safety cap of **10,000 iterations** so an accidental infinite loop cannot freeze the game.

### 12. Command Reference

| Command | Example | Effect |
|---|---|---|
| `move <dir>` | `move left` | Move the acting entity in `left`, `right`, `up`, or `down` at its `speed`. `<dir>` may be a variable: `move $dir` |
| `jump` | `jump` | Apply the player's `jump_force` impulse (works when grounded) |
| `move_toward <Target>` | `Hunter1.move_toward Player` | **New.** Moves an entity toward a target: reads both positions, sets horizontal velocity toward it, and tells patrol AI to yield for 1.5 seconds so patrolling does not overwrite the chase |
| `rotate <degrees>` | `rotate 180` | Set the facing direction. `0` faces right, `180` faces left. Used to point bullets and flip the sprite |
| `gravity on/off` | `gravity off` | Toggle gravity |
| `apply_force <fx> <fy>` | `apply_force 0 -300` | Add a force to the entity's rigid body |
| `play <sound>` | `play pickup` | Play a named sound effect |
| `spawn <Blueprint>` | `spawn PlayerBullet` | Create a copy of a blueprint at the acting entity |
| `spawn <Blueprint> at <x> <y>` | `spawn Boss at 1000 -80` | Create a copy at a world position. `x` and `y` can be variables (`at $ex 100`) |
| `<Entity>.spawn <Blueprint>` | `TurretLeft.spawn EnemyBullet` | Make a **specific named entity** spawn the blueprint (so a turret fires from the turret) |
| `despawn` | `despawn` | Remove the entity involved in the event (for example the coin that was touched) |
| `heal <Target> <n>` | `heal Player 2` | Restore `n` health to the target |
| `damage <Target> <n>` | `damage Player 1` | Deal `n` damage to the target |
| `damage <n>` | `damage 1` | Deal `n` damage to the other entity involved in the collision |
| `look_at <Entity> <Target>` | `look_at TurretLeft Player` | Rotate an entity to face another, used to aim turrets |
| `victory` | `victory` | End the level in the victory state |

Statements that are not commands: `set`, `if`, `else`, `while`, `for`.

### 13. Targets and Matching

`on collide X`, `heal X`, `damage X`, and `look_at A X` all take a **target**. A target is matched against an entity's **name**, **type**, or **tag**. That makes it easy to write one rule for a whole family:

```argdl
on collide Coin        // matches Coin1, Coin2, ... Coin9, by name prefix and tag
{
    despawn
    heal Player 2
}

on collide Enemy       // matches every Enemy: patrolling grunts, brutes, hunters, spawned grunts
{
    damage Player 1
}

on collide Turret      // matches anything with  tag : Turret
{
    damage Player 1
}
```

Use `#Name` to identify **one** entity (`TurretLeft`), and `tag` to group **different** entities under one label (`TurretLeft`, `TurretMid`, and `TurretRight` all share `tag : Turret`).

### 14. Coordinate System

- Coordinates are in world units, with the origin at the **top-left** of the world.
- **+x goes right, +y goes down.** That is why the ground sits near `y : 580` while the sky is near `y : 0`, and why falling meteors spawn at negative `y` (above the screen) with `projectile_angle : 90`.
- `x` and `y` are the entity's **center**. A platform with `x : 400`, `width : 800` covers `x = 0` to `800`.
- Backgrounds are usually larger than the screen and centered in the world.

### Walkthrough: A Minimal Level

A complete, tiny, playable level using v2 features: a blueprint, instances, a variable, a guard, and a win condition.

```argdl
// ---- Templates ----
#Platform Blueprint {
    color : green
    sprite : "assets/dirt.jpg"
    kinematic : on          // solid, never moves
    collider : box
}

#Coin Blueprint {
    size : 16
    sprite : "assets/coin.png"
    gravity : off           // floats in place
    trigger : on            // overlap detected, no physical bump
    collider : box
    tag : Coin
}

// ---- Player (unique object, owns the score variable) ----
#Player Character {
    x : 100
    y : 500
    size : 32
    sprite : "assets/character.jpg"
    speed : 320
    jump_force : 500
    mass : 1
    gravity : on
    friction : 0.4
    collider : box
    coins : 0               // variable
}

// ---- World (one line each) ----
#Floor Platform at 400 580 {
    width : 800
    height : 40
}

#Coin1 Coin at 300 540
#Coin2 Coin at 450 540
#Coin3 Coin at 600 540

// ---- Controls ----
on a     { move left  }
on d     { move right }
on space { jump       }

// ---- Rules ----
on collide Coin
{
    despawn
    set coins = $coins + 1
    play pickup

    if $coins == 3 {
        victory             // collected all three
    }
}
```

What happens: the interpreter resolves the blueprints, instantiates the floor and the coins, and registers the handlers. The Player falls and lands on the Floor. Holding `D` runs `move right` each frame. Touching a coin fires `on collide Coin` (all three coins match the target `Coin`), which removes it, adds 1 to `coins`, and on the third coin ends the level.

### Recipes

**Collectible that heals, with a lucky bonus**

```argdl
#Coin Blueprint { size : 16  gravity : off  trigger : on  collider : box  tag : Coin }
#Coin1 Coin at 250 380

on collide Coin if $coins < 4 {
    despawn
    set coins = $coins + 1
    heal Player 2
    play pickup

    set lucky = random 1 3
    if $lucky > 2 {
        heal Player 2
    }
}
```

**Patrolling enemy from a blueprint**

```argdl
#Enemy Blueprint {
    size : 32
    gravity : on
    health : 3
    collider : box
    tag : Enemy
    patrol_speed : 70
}

#Grunt Blueprint extends Enemy {
    health : 2
    patrol_min : 200      // walks back and forth between x = 200 ...
    patrol_max : 700      // ... and x = 700
}

#Enemy1 Grunt at 400 540
```

**Enemy that chases only when you are close**

```argdl
on timer 0.5 {
    if distance_to Hunter1 < 500 {
        Hunter1.move_toward Player
    }
}
```

**Safe boss chase (the boss may not exist yet, or may already be dead)**

```argdl
on timer 0.5 {
    if exists Boss {
        if distance_to Boss < 900 {
            Boss.move_toward Player
        }
    }
}
```

**Turret that aims and fires on a timer**

```argdl
#Turret Blueprint {
    size : 40
    mass : 999
    gravity : off
    kinematic : on
    health : 8
    tag : Turret
    collider : box
}

#EnemyBullet Blueprint {
    tag : EnemyBullet
    size : 12
    sprite : "assets/laserRed01.png"
    physics : off
    collider : box
    projectile_speed : 340
    projectile_lifetime : 3.0
    projectile_damage : 1
}

#TurretLeft Turret at 350 436

on timer 4
{
    look_at TurretLeft Player              // aim
    if distance_to TurretLeft < 800 {      // only fire when the player is in range
        TurretLeft.spawn EnemyBullet       // fire, from the turret itself
    }
}
```

**Shooting combo: every third shot is a heavy shot**

```argdl
on j {
    spawn PlayerBullet
    set combo = $combo + 1
    if $combo >= 3 {
        spawn HeavyBullet
        set combo = 0
    }
}

on collide Enemy {
    damage Player 1
    damage 1
    set combo = 0            // getting hit resets the combo
}
```

**Waves that escalate over time**

```argdl
on timer 1 {
    set survived = $survived + 1
    if $survived == 10 { set wave = 2 }
    if $survived == 20 { set wave = 3 }
    if $survived == 30 { set wave = 4 }
}

on timer 15 if $wave >= 2 {
    spawn Grunt at 300 100
    spawn Grunt at 1700 100
}

on timer 8 if $wave >= 3 {
    spawn Brute at 700 100
}

on timer 9 if $wave >= 4 {
    set ex = random 300 1700
    spawn Hunter at $ex 100
}
```

**Timed enemy spawner**

```argdl
#Spawner Blueprint {
    size : 28
    mass : 999
    gravity : off
    kinematic : on
    health : 6
    tag : Spawner
    skip_collision : on
    spawn_at_self : on
}

#SpawnerLeft Spawner at 150 100 {
    spawn_every : 10             // a new grunt every 10 seconds
    spawn_prefab : Grunt
}
```

**Falling hazard (meteors)**

```argdl
#Meteor Blueprint extends Bullet {
    tag : Meteor
    size : 36
    projectile_speed : 280
    projectile_lifetime : 6.0
    projectile_damage : 2
    projectile_angle : 90              // straight down (+y is down)
    projectile_friendly_tag : Enemy    // does not hurt enemies
}

#BigMeteor Blueprint extends Meteor {
    size : 60
    projectile_speed : 200
    projectile_damage : 4
}

on timer 6  { spawn Meteor at 400 -60 }       // starts above the screen
on timer 10 { spawn BigMeteor at 700 -80 }
```

**Boss after a delay**

```argdl
#Boss Blueprint {
    size : 80
    mass : 6
    gravity : on
    collider : box
    tag : Boss
    boss : on
    hp : 30
}

after 40 {
    spawn Boss at 1000 -80
    play hurt
}
```

**Win condition that depends on state**

```argdl
#Portal Blueprint { size : 60  gravity : off  trigger : on  collider : box  tag : Portal }
#ExitPortal Portal at 1600 500

on collide Portal if $survived >= 60 { victory  play pickup }   // survived long enough: win
on collide Portal if $survived < 60  { play hurt }              // too early: refused
```

**Facing direction plus multiple key bindings**

```argdl
on a     { move left   rotate 180 }
on left  { move left   rotate 180 }
on d     { move right  rotate 0   }
on right { move right  rotate 0   }
```

**Variable direction**

```argdl
set dir = "left"
move $dir
```

### The Bundled Example Games

The bundled levels deliberately use different subsets of the language so you can learn from each.

| Level | Theme | Win / lose | ARGDL features it demonstrates |
|---|---|---|---|
| **Platformer** | Mario-style side scroller, 3000 units wide | Reach the flagpole; falling below the platforms is game over | Wide parallax `Background`, many `Platform` entities (ground, pipes, floating platforms, stairs, flagpole), trigger `Item` coins, `Enemy` entities, physics crates (`mass`, `restitution`), keyboard-only input handlers |
| **Turret Arena** | Single-screen space arena | Survive: defeat enemies while turrets fire and a boss arrives at 60 s | `health`, `invuln_duration`, `knockback`, bullet templates, `on j { spawn ... }` shooting, turrets aimed with `look_at` and fired on `on timer`, `spawn_every` spawners, `patrol_*` enemies, `on collide` damage and healing, timed boss spawn |
| **Meteor Run** | Wider space level with a goal | Reach the `ExitPortal` to win; health is limited to 6 | Everything above, plus falling meteors (`projectile_angle`, `projectile_friendly_tag`), several concurrent `on timer` handlers, `after` one-shot spawns, an `on collide Portal { victory }` win condition |
| **Wave Survival** (`game.argdl`) | Wide arena (about 2000 units) with floating platforms, turrets, spawners, and a portal | Survive at least 60 seconds, then reach the `ExitPortal`. Dying is game over | **Full v2 showcase**: multi-level blueprint inheritance, one-line instances, per-instance overrides, variables, guards, `if` / `else`, `random`, `distance_to`, `exists`, `move_toward`, escalating waves, and a shooting combo |

Suggested reading order: the platformer (pure layout and physics), then the Turret Arena (combat and timers), then the Meteor Run (a complete objective-driven level), then Wave Survival (state, logic, and inheritance).

#### Wave Survival in detail

**World**
- A parallax space `Backdrop`, left and right walls, two ground sections, and six floating platforms built from `Platform` / `SmallPlatform` blueprints
- Nine coins: seven regular `Coin` blueprints and two `BigCoin` blueprints that extend `Coin` (bigger, heal more)
- Three turrets (`Turret` x 2 and a tougher `HeavyTurret` that extends it)
- Two enemy spawners that periodically produce grunts
- A starting set of enemies: two `Grunt`, one `Brute`, one `Hunter`
- An `ExitPortal` that only works after the survival timer reaches 60

**Blueprint hierarchy used**

```
Platform ─► SmallPlatform
Coin ─► BigCoin
Turret ─► HeavyTurret
Bullet ─┬─► PlayerBullet ─► HeavyBullet
        ├─► EnemyBullet
        └─► Meteor ─► BigMeteor
Enemy ─┬─► Grunt
       ├─► Brute
       └─► Hunter
Boss, Spawner, Portal (standalone)
```

**State kept on the Player**: `coins`, `score`, `wave`, `survived`, `combo`.

**What the rules do**
- **Controls**: A / D / arrows move and face the right way, W / Up / Space jump, J shoots
- **Combo**: every third shot also fires a `HeavyBullet` and resets the counter. Being hit by an enemy, turret, meteor, or boss resets the combo
- **Pickups**: `Coin` heals 2 and `BigCoin` heals 5, both add to `coins`
- **Damage**: enemies, turrets, meteors, and the boss each hurt the player by a different amount
- **Clock**: a 1-second timer increases `score` and `survived`, and moves `wave` up at 10, 20, 30, and 40 seconds
- **Chasing**: every half second, the Hunter chases the player when within 500 units, and the Boss chases when within 900
- **Turrets**: each turret turns toward the player and fires on its own timer, only when the player is in range (the heavy turret fires twice)
- **Meteors**: small and large meteors fall from above on separate timers
- **Waves**: more grunts at wave 2, brutes at wave 3, randomly placed hunters at wave 4, and bigger mixed groups at wave 5
- **Boss**: arrives 40 seconds in (`after 40`)
- **Win**: touch the portal once `survived >= 60`. Touch it too early and you only hear a hurt sound

### Migrating from ARGDL v1

Existing v1 scripts keep their general shape. Entity blocks such as `#Player Character { ... }` are unchanged, and handlers still work. To take advantage of v2:

1. **Turn `Prefab` blocks into `Blueprint` blocks.** Where one prefab repeated another's properties, use `extends` and keep only the differences.
2. **Replace repeated entity blocks with instances.** Anything that was copy-pasted with different `x` / `y` becomes `#Name Blueprint at x y`.
3. **Keep `spawn` as it was.** `spawn <Name>` now looks the name up in the blueprint registry.
4. **Move repeated numbers into variables** when you want logic (`coins`, `score`, `wave`).
5. **Add guards instead of duplicating handlers** when behavior should depend on state.
6. **Use `exists` before commands that target an entity that might be gone.**

### Tips and Gotchas

- **Names must be unique.** Handlers such as `TurretLeft.spawn` and `look_at TurretLeft Player` rely on exact entity names.
- **Repeated things share a name prefix or a tag** (`Coin1`..`Coin9`), so one `on collide Coin` handler covers them all.
- **Use `trigger : on` for pickups and goals** so the player passes through them instead of bumping into them.
- **Use `kinematic : on` for anything solid that should stay put**: platforms, turrets, spawners.
- **Turrets and other immovable objects** work best with a very large `mass` (for example `999`) in addition to `kinematic : on`.
- **Set `physics : off` and `collider : box` on projectiles** so they fly straight instead of falling or being pushed.
- **Remember +y is down.** A projectile angle of `90` is straight down, and spawning at a negative `y` places something above the visible screen.
- **Sprite paths are relative to the working directory**, so keep the `assets/` folder next to the executable.
- **Every entity that should collide needs `collider : box`.** Entities with `skip_collision : on` are ignored by collision entirely.
- **Timers repeat, `after` fires once.** Use `after` for one-time events and `on timer` for repeating ones.
- **Guard your chases with `exists`.** A timer keeps firing after the entity it targets has died.
- **Integer math truncates.** `10 / 3` is `3`. Use a decimal (`10 / 3.0`) when you need a fractional result.
- **`for` variables are not local.** The loop variable stays on the entity after the loop ends. Pick names that will not collide with real variables.
- **Compare against a variable with `$`.** `$coins` reads the variable. A bare `coins` is just a word.
- **Blueprint cycles are errors.** `A extends B` and `B extends A` will be rejected when the level loads.
- **Parse errors stop loading.** Check the console for the error message if your level does not start.
- **Loops are capped at 10,000 iterations**, so a runaway `while` will stop rather than freeze the game.

### Current Limitations

ARGDL is capable of non-trivial gameplay, but it is intentionally small. Not yet available:

- **Custom functions**: you cannot define `spawn_wave(n)` and call it from several places. Logic is written inline.
- **Arrays and collections**: no `[Grunt, Brute, Boss]`, so lists of things are written out as separate lines.
- **Local scope**: every variable belongs to its entity, and loop variables leak.
- **A `wait` command**: timed sequences inside one event must be built from timers and `after`.
- **Cross-entity iteration**: you can read `Enemy1.health`, but you cannot say "for each enemy, do X". You name each one.
- **Type annotations**: everything is dynamically typed.
- **String methods and a math library**: no `abs`, `min`, `max`, `sqrt`, `sin`, `upper`, `contains`, and so on.

Engine-side limitations:

- Single-threaded, with O(n²) collision detection and no spatial partitioning
- All assets are loaded up front (no streaming)
- No built-in animation system, particle system, or tilemaps
- No save / load, no networking
- The UI (health bar, banners, coin counter) is built in rather than scriptable
- No visual editor, and scripts are re-read by restarting the game rather than hot-reloading

---

## Technical Details

### Physics System
- Rigid body dynamics with impulse-based collision resolution
- AABB (Axis-Aligned Bounding Box) collision detection with axis-separated resolution
- Ground detection and friction
- Support for static, kinematic, and dynamic bodies
- Layer-based collision filtering
- Trigger volumes that report overlaps without physical response
- New contacts are tracked by pair, so `on collide` fires once per new contact rather than every frame

### Rendering Pipeline
- OpenGL 3.3 Core Profile
- Custom shader programs for sprite rendering
- Texture loading with stb_image, cached by path
- Parallax background support
- Layer-based rendering order

### Language Implementation
- **Lexer**: numbers, strings, identifiers, punctuation, and the full operator set, plus `#` and `$`
- **Parser**: recursive descent. Produces objects, blueprints, instances, events, statements, and expression trees. Operator precedence is encoded in the expression grammar
- **Statements** (`Statement.h`): the node types used inside event bodies: command, `set`, `if` / `else`, `while`, and `for`. Each carries its own condition, body, and else-body where needed
- **Value system**: a tagged value (int, float, bool, string, nil) with coercion helpers (`AsBool`, `AsNumber`, `AsInt`, `AsString`)
- **Expression evaluator**: literals, variables, member access, function calls, binary and unary operators
- **Interpreter**: blueprint resolution with parent-first inheritance, cycle detection, and caching; instance creation with overrides
- **RuntimeBridge**: builds entities from script objects, attaches components by property, executes statements, evaluates guards, dispatches commands, and runs key, timer, and collision events
- **Timers**: repeating timers accumulate time and fire when the period is reached; `after` timers fire once and are remembered so they never repeat
- **Safety**: 10,000-iteration cap on `while`, and cycle detection on inheritance

### Architecture
- Entity-Component pattern for flexibility
- Component-based physics and rendering
- Script-driven game logic via ARGDL
- Clear separation between engine systems and game content
- Lexer, Parser, Interpreter, and RuntimeBridge pipeline for the scripting language

### Performance Notes
- Blueprint resolution is cached, so inheritance costs nothing after load
- Texture and sound caching
- Guards prevent unnecessary handler work
- Collision pair tracking avoids duplicate events

## Controls

The keyboard controls are defined in the ARGDL script (`on <key>` handlers), so they can be changed without touching C++. The bundled levels use:

- **A / Left Arrow**: Move left
- **D / Right Arrow**: Move right
- **W / Up Arrow / Space**: Jump
- **J**: Shoot (combat levels; every third shot is a heavy shot in Wave Survival)
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
- Collect coins to increase score and restore health
- Avoid or shoot enemies to defeat them
- Reach the flagpole (platformer) or the exit portal (Meteor Run, Wave Survival) to complete the level
- Falling below platforms results in game over
- In combat levels, the player has limited health, brief invulnerability after each hit, and knockback when hurt
- In Wave Survival, enemies escalate as the survival timer grows, and the portal only opens for you after 60 seconds
- Camera follows the player smoothly

## Roadmap

This is V1 of the engine, and there are plans to take it much further. Ideas under consideration:

**Language**
- Arrays and lists
- Custom function definitions
- Local variable scope
- A `wait` command for sequencing
- Cross-entity iteration ("for each enemy")
- A small standard library (`abs`, `min`, `max`, `sqrt`, ...)
- Better error messages with more context

**Engine**
- Spatial partitioning for collision
- Particle system
- Sprite-sheet animation
- Save / load game state
- Tilemap support
- Music and audio streaming
- A scriptable UI system (buttons, text, sliders)

**Tooling**
- Hot-reload of `.argdl` files without restarting
- Syntax highlighting for `.argdl`
- A command-line script validator
- A visual level editor

## License

This project is licensed under the [MIT License](LICENSE).

## Credits

Built with C++, OpenGL, SDL2, and custom game engine architecture. Designed and written by Aayush Jha ([@Aayushjha11h](https://github.com/Aayushjha11h)).
