# Open Tournament

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
* Follow the [Setting Up Open Tournament Guide](#setting-up-open-tournament)
* Join [Discord](https://discord.gg/Rw8evNT) for real-time communication and collaboration.
* Check [Trello](https://trello.com/b/RviM3J39/open-tournament-public-taskboard) and our [Issues](https://github.com/OpenTournament/OpenTournament/issues)

## Additional Links

* Official Website: [www.opentournamentgame.com](https://www.opentournamentgame.com)
* Additional discussion can be found on www.reddit.com/r/OpenTournament

_______________________________________________________________________________________

# Setting Up Open Tournament

## What You Need to Get Started

* [Epic Games Launcher](https://www.epicgames.com/store/download)
* [Git Client](#git-client)
* [Project Prerequisites](#project-prerequisites)
* [Visual Studio 2017 or later](#visual-studio)

### Epic Games Launcher

The Open Tournament project is built using 4.24 versions of Unreal Engine 4. Please ensure that you have the correct engine version installed.

* The correct engine version can be determined by looking at the [OpenTournament.uproject](https://github.com/OpenTournament/OpenTournament/blob/develop/OpenTournament.uproject#L3) from within a text editor. Find the line which says "EngineAssociation" and note the numerical value associated with it.
* The correct Unreal Engine version can be installed through the Epic Games Launcher. On the sidebar, select "Unreal Engine" then select the "Library" tab at the top. Select the "+" button next to Engine Versions and set it to the appropriate Engine version.

### Git Client

Git is a version control system utilized by GitHub & the Open Tournament project. The Git tool can be interfaced through using the command line, or using graphical frontends such as GitKraken, Git Fork or Sourcetree. Please refer to Git, GitHub, or your GUI client's tutorials on how to clone the Open Tournament repository.


### Project Prerequisites

The Open Tournament project has certain dependencies on assets that we didn't include in our GitHub repository for size considerations. The prerequisites can be downloaded via the batch file in `<OpenTournament>\otscripts\OTSyncAssets.bat`

This part of our pipeline is still being figured out. If you have trouble, please reach out on Discord! 

### Visual Studio

* Refer to the [Unreal Engine 4 Documentation](
https://docs.unrealengine.com/en-US/Programming/Development/VisualStudioSetup/index.html) prior to setting up Visual Studio
* Download and install [Visual Studio](https://visualstudio.microsoft.com) 2017 or 2019
* Within your cloned Open Tournament repository, right click on the OpenTournament.uproject
* Select "Generate Visual Studio project Files"
* Open the OpenTournament.sln file in Visual Studio
* Build the solution by opening the "Build" menu dropdown and selecting Build Solution
* Once your initial project build is complete you should be able to double click on OpenTournament.uproject to open the project. This will display the Unreal Engine 4 Editor.


# Code and Asset Style Guide

Code and Asset style, formatting, and organization is an important task and is the responsibility of everyone contributing to the project. In order to ensure that your contributions are considered, please follow the following guidelines. If questions arise, defer to the conventions used in the OpenTournament project currently or decide upon & document new conventions.

* [Code Style Guide](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/index.html) - Please follow the Epic style guidelines for most code submissions, except where otherwise specified.
* [Asset Style & Naming Guide](https://github.com/Allar/ue4-style-guide) - Please follow these guidelines for Blueprints and other asset types
