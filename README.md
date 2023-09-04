# Open Tournament

> Important: If you cloned this repository prior to 2-Aug-20, please make sure you have git lfs installed and re-clone to a new location 

> Important: If you are cloning this repository to contribute, please ensure to utilize the `unstable` branch as the base as this is the most currently up-to-date branch


## Overview

The **Open Tournament Project** is a community-led effort to develop a new game that will recreate and expand upon the formula of Unreal Tournament, Quake, and other classic FPS games.

## Legal

* Code is (c) 2019-2023 Open Tournament Team. Usage is subject to our license.
* Other content may fall under different licenses and is used with permission only for Open Tournament.
* This is a public repository with access available for contributors to Open Tournament.
* Please do not share or redistribute in any form without approval from project leadership.

## Getting Started

The **Open Tournament Project** is an experiment in open game development. The project is run by volunteer efforts from veteran and amateur developers. Our process is itself a work in progress, so bear with us as we find out how to best onboard and collaborate effectively.

* [Register as a Developer](https://forms.gle/s7qcsKLuVVm1WzgB7). This helps us keep track of developer interest, skillset, and contact information
* Follow the [Setting Up Open Tournament Guide](#SettingUpOpenTournament)
* Join [Discord](https://discord.gg/Rw8evNT) for real-time communication and collaboration.
* Check our [Issues](https://github.com/OpenTournament/OpenTournament/issues) or our [Trello](https://trello.com/b/RviM3J39/open-tournament-public-taskboard)

## Additional Links

* Official Website: [www.opentournamentgame.com](https://www.opentournamentgame.com)
* Additional discussion can be found on [r/OpenTournament](www.reddit.com/r/OpenTournament)
* Twitter: [Open Tournament](www.twitter.com/open_tournament)

_______________________________________________________________________________________

# Setting Up Open Tournament

## What You Need to Get Started

* GitHub Account
* Epic Games Launcher
* Git Client
* Visual Studio 2022 (*C++ Developer Only*)

### GitHub Account

If you're reading this there's a good chance you already have a GitHub account, but if you don't then create an account on GitHub! 

### Epic Games Launcher

The Open Tournament project is built using Unreal Engine 5.1. Please ensure that you have the correct engine version installed.

* The correct engine version can always be determined by looking at the `OpenTournament.uproject` from within a text editor. Find the line which says "EngineAssociation" and note the numerical value associated with it.
* The correct Unreal Engine version can be installed through the Epic Games Launcher. On the sidebar, select "Unreal Engine" then select the "Library" tab at the top. Select the "+" button next to Engine Versions and set it to the appropriate Engine version.

### Git Client

Git is a version control system utilized by GitHub & the Open Tournament project. The Git tool can be interfaced through using the command line, or using graphical frontends such as GitKraken, Git Fork or Sourcetree. Make sure you install Git with LFS support. Please refer to Git, GitHub, or your GUI client's tutorials on how to clone the Open Tournament repository.

### Visual Studio

> NOTE: If you are not a C++ developer, you should not need Visual Studio or any other C++ IDE. Double-click OpenTournament.uproject, and report back to us any issue in Discord.

* Refer to the [Unreal Engine Documentation](
https://docs.unrealengine.com/en-US/Programming/Development/VisualStudioSetup/index.html) prior to setting up Visual Studio
* Download and Install Visual Studio 2019
* Within your cloned Open Tournament repository, right click on the OpenTournament.uproject
* Select "Generate Visual Studio project Files"
* Open the OpenTournament.sln file in Visual Studio
* Set solution configuration to `Development Editor` and platform to `Win64`
* Build the solution by opening the "Build" menu dropdown and selecting Build
* Once your initial project build is complete you should be able to double click on OpenTournament.uproject to open the project. This will display the Unreal Engine 4 Editor.

_______________________________________________________________________________________

## Getting Started with Open Tournament

### Creating a GitHub Fork

The first step to getting started developing for Open Tournament is to [create a fork](https://guides.github.com/activities/forking/) of the Open Tournament repository on GitHub. This will create a copy of the repository to your GitHub account.

### Clone the Forked Repository

The second step, after you have created a fork, is to [clone the fork](https://guides.github.com/activities/forking/). Cloning will create a copy of the repository on your computer. Once you have completed this step, you should be able to navigate to the location on your computer where the repository resides and see the Open Tournament project.

### Run OpenTournament.uproject

Navigate to the location where you cloned the Open Tournament repository and find OpenTournament.uproject. You can run the Open Tournament editor by double clicking the OpenTournament.uproject file.

_______________________________________________________________________________________

## Additional Recommended Steps

### Set the Upstream Repository

In the Git commandline, execute the following command:  
    git remote add upstream https://github.com/OpenTournament/OpenTournament.git

### Get Latest from the Official Open Tournament Repository

You will occasionally want to ensure that you have the latest updates from the official Open Tournament repository.

* Ensure that you are on your local 'master' branch.
* Ensure that you stash or revert any local changes.

In the Git commandline, execute the following command:  
    git rebase upstream/master

_______________________________________________________________________________________

## How to Contribute

Contributing to Open Tournament is done through GitHub, using the Pull Request (PR) system.

Here is a simple [Guide to Submitting Pull Request](https://www.freecodecamp.org/news/how-to-make-your-first-pull-request-on-github-3/)

In the future, we will have a more defined process & workflow for certain types of content. However, for now here are some basic guidelines for pull requests:
* Reference an [Issue](https://github.com/OpenTournament/OpenTournament/issues) in every PR. Format should be OT-<IssueNumber>
* Try to make it easy to understand both the intent of your changes and the changes themselves.
* Limit Scope of PRs as much as possible
* If your PR contains any blueprint changes, please provide before/after screenshots of any changed logic.
* If your PR contains any blueprint changes, but these changes are value-only, a detailed summary of changes made (Old Value -> New Value) is sufficient.
* If your PR contains edits to materials, textures, or other visual content please provide screenshots & detailed summary of changes.
* If your PR contains edits to maps, please provide screenshots & detailed summary of changes.

Unfortunately while there is extra effort on the developer side, this is needed in order to keep the PR and review process manageable!

_______________________________________________________________________________________
	
# Code and Asset Style Guide

Code and Asset style, formatting, and organization is an important task and is the responsibility of everyone contributing to the project. In order to ensure that your contributions are considered, please follow the following guidelines. If questions arise, defer to the conventions used in the OpenTournament project currently or decide upon & document new conventions.

* [Code Style Guide](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/index.html) - Please follow the Epic style guidelines for most code submissions, except where otherwise specified. 
* [Asset Style & Naming Guide](https://github.com/Allar/ue5-style-guide) - Please follow these guidelines for Blueprints and other asset types
* Follow the existing naming conventions where possible, or discuss!
