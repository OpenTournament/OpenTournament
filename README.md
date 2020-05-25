# OpenTournament

The Open Tournament project is a community-led effort to develop a game that will recreate and expand upon the formula of Unreal Tournament, Quake, and other classic FPS gameplay.

* We are working towards a public build release with a more representative sample of features and content
* We are also working on both an internal and external roadmap!
* This is a public repository with access available for contributors to Open Tournament.
* Code is (c) 2019-2020 Open Tournament Team. Usage of code is subject to our license.
* Please do not share code or content without discussing with other project members.
* Official webpage : [www.opentournamentgame.com](https://www.opentournamentgame.com)
* Additional discussion can be found on www.reddit.com/r/OpenTournament

# Setting Up OpenTournament

## What You Need to Get Started

* Epic Games Launcher
* Git Client
* Project Prerequisites
* Visual Studio 2017 or Later

### Epic Games Launcher

The Open Tournament project is built using 4.24 (or later) versions of Unreal Engine 4. Please ensure that you have the correct engine version installed.

* The correct engine version can be determined by looking at the OpenTournament.uproject from within a text editor. Find the line which says "EngineAssociation" and note the numerical value associated with it.
* The correct Unreal Engine version can be installed through the Epic Games Launcher. On the sidebar, select "Unreal Engine" then select the "Library" tab at the top. Select the "+" button next to Engine Versions and set it to the appropriate Engine version.

### Git Client

Git is a version control system utilized by GitHub & the Open Tournament project. The Git tool can be interfaced through using the command line, or using graphical frontends such as GitKraken, Git Fork or Sourcetree. Please refer to Git, GitHub, or your GUI client's tutorials on how to clone the Open Tournament repository.


### Project Prerequisites

The Open Tournament project has certain dependencies on assets that we didn't include in our GitHub repository for size considerations. The prerequisites can be downloaded via the batch file in <OpenTournament>\otscripts\OTSyncAssets.bat

This part of our pipeline is still being figured out. If you have trouble, please reach out on Discord! 

### Visual Studio

* Refer to the [Unreal Engine 4 Documentation](
https://docs.unrealengine.com/en-US/Programming/Development/VisualStudioSetup/index.html) prior to setting up Visual Studio
* Download and Install Visual Studio 2017 or 2019
* Within your cloned Open Tournament repository, right click on the OpenTournament.uproject
* Select "Generate Visual Studio project Files"
* Open the OpenTournament.sln file in Visual Studio
* Build the solution by opening the "Build" menu dropdown and selecting Build
* Once your initial project build is complete you should be able to double click on OpenTournament.uproject to open the project. This will display the Unreal Engine 4 Editor.


# Code and Asset Style Guide

Code and Asset style, formatting, and organization is an important task and is the responsibility of everyone contributing to the project. In order to ensure that your contributions are considered, please follow the following guidelines. If questions arise, defer to the conventions used in the OpenTournament project currently or decide upon & document new conventions.

* [Code Style Guide](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/index.html) - Please follow the Epic style guidelines for most code submissions, except where otherwise specified.
* [Asset Style & Naming Guide](https://github.com/Allar/ue4-style-guide/blob/master/README.md) - Please follow these guidelines for Blueprints and other asset types
