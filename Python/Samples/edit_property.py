# Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#

import unreal

obj = unreal.MediaPlayer()

# do NOT run the special code in the background
obj.play_on_open = True
print obj.play_on_open

# do run the special code in the background
obj.set_editor_property("play_on_open", True)
print obj.get_editor_property("play_on_open")

