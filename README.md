# Open Tournament

## Important: If you cloned this repository prior to 2-Aug-20, please make sure you have git lfs installed and re-clone to a new location 

## Overview

The **Open Tournament Project** is a community-led effort to develop a new game that will recreate and expand upon the formula of Unreal Tournament, Quake, and other classic FPS games.

## Legal

* Code is (c) 2019-2020 Open Tournament Team. Usage is subject to our license.
* Other content may fall under different licenses and is used with permission only for Open Tournament.
* This is a public repository with access available for contributors to Open Tournament.
* Please do not share or redistribute in any form without approval from project leadership.

## Getting Started

The **Open Tournament Project** is an experiment in open game development. The project is run by volunteer efforts from veteran and amateur developers. Our process is itself a work in progress, so bear with us as we find out how to best onboard and collaborate effectively.

* [Register as a Developer](https://forms.gle/s7qcsKLuVVm1WzgB7). This helps us keep track of developer interest, skillset, and contact information
* Follow the [Setting Up Open Tournament Guide](# Setting Up Open Tournament)
* Join [Discord](https://discord.gg/Rw8evNT) for real-time communication and collaboration.
* Check [Trello](https://trello.com/b/RviM3J39/open-tournament-public-taskboard) and our [Issues](https://github.com/OpenTournament/OpenTournament/issues)

## Additional Links

* Official Website: [www.opentournamentgame.com](https://www.opentournamentgame.com)
* Additional discussion can be found on www.reddit.com/r/OpenTournament

_______________________________________________________________________________________

# Setting Up Open Tournament

## What You Need to Get Started

* Epic Games Launcher
* Git Client
* Visual Studio 2019 (*)

### Epic Games Launcher

The Open Tournament project is built using Unreal Engine 4.25. Please ensure that you have the correct engine version installed.

* The correct engine version can be determined by looking at the OpenTournament.uproject from within a text editor. Find the line which says "EngineAssociation" and note the numerical value associated with it.
* The correct Unreal Engine version can be installed through the Epic Games Launcher. On the sidebar, select "Unreal Engine" then select the "Library" tab at the top. Select the "+" button next to Engine Versions and set it to the appropriate Engine version.

### Git Client

Git is a version control system utilized by GitHub & the Open Tournament project. The Git tool can be interfaced through using the command line, or using graphical frontends such as GitKraken, Git Fork or Sourcetree. Make sure you install Git with LFS support. Please refer to Git, GitHub, or your GUI client's tutorials on how to clone the Open Tournament repository.

### Visual Studio

NOTE : Lately we have been experimenting with including precompiled binaries in the repository, to ease the setup for non-developers. You should now be able to run Editor straight after cloning, given you have the right UnrealEngine version installed from Epic Games Launcher. If you are not a C++ developer, you can try skipping this step, double-click OpenTournament.uproject, and report back to us any issue in Discord.

* Refer to the [Unreal Engine 4 Documentation](
https://docs.unrealengine.com/en-US/Programming/Development/VisualStudioSetup/index.html) prior to setting up Visual Studio
* Download and Install Visual Studio 2019
* Within your cloned Open Tournament repository, right click on the OpenTournament.uproject
* Select "Generate Visual Studio project Files"
* Open the OpenTournament.sln file in Visual Studio
* Set solution configuration to `Development Editor` and platform to `Win64`
* Build the solution by opening the "Build" menu dropdown and selecting Build
* Once your initial project build is complete you should be able to double click on OpenTournament.uproject to open the project. This will display the Unreal Engine 4 Editor.

# Code and Asset Style Guide

Code and Asset style, formatting, and organization is an important task and is the responsibility of everyone contributing to the project. In order to ensure that your contributions are considered, please follow the following guidelines. If questions arise, defer to the conventions used in the OpenTournament project currently or decide upon & document new conventions.

* [Code Style Guide](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/index.html) - Please follow the Epic style guidelines for most code submissions, except where otherwise specified.
* [Asset Style & Naming Guide](https://github.com/Allar/ue4-style-guide/blob/master/README.md) - Please follow these guidelines for Blueprints and other asset types
