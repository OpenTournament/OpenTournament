# Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#

import unreal

# create a float 3 with null value
v1 = unreal.Vector()

# assignment
v1.x = 0

# create a float 3 with x = 10, y = 20, z = 30
v2 = unreal.Vector(10, 20, 30)

# math operation
v3 = (v1 + v2) * 2
print v3
