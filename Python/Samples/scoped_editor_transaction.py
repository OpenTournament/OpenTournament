# Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#

import unreal

obj = unreal.MediaPlayer()

with unreal.ScopedEditorTransaction("My Transaction Test") as trans:
	obj.set_editor_property("play_on_open", True)
	obj.set_editor_property("vertical_field_of_view", 60)
