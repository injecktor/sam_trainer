# sam_trainer

Internal trainer for **Serious Sam 2** with ImGui. 
Working only if fullscreen setting set to **no**

## Functionality

Trainer menu allows you:
- Make Sam invincible by hp or armor
- Set current hp and armor
- Set max hp and armor

## How to use

Open project with Visual Studio 2022 and compile 32-bit dll.
Then inject in the game.

## How it works

### Game functions handling

Trainer searches game functions by signature scan and checks the entity pointer whether it's a player or not.

### ImGui

ImGui draws on window that covers sam window due to imgui doesn't support DirectX8