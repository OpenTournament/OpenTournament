# TextureSettings.py
# This is a script to find Texture2D assets intended for UI and properly set the following properties:
# LOD Group, Compression Settings, Mip Gen Settings, Never Stream
#
# Reference: https://api.unrealengine.com/INT/PythonAPI/

import unreal

aregistry = None
aregistry = unreal.AssetRegistryHelpers.get_asset_registry()

# Test if AssetRegistry was Found
#if aregistry is not None:
#    print "Got AssetRegistry"
    
# Find Assets in a given Path
assetlist = aregistry.get_assets_by_path("/Game/Sprites/UI/Icons", True, True)

# Detect if our AssetPath was valid / Assets were found
#print len(uitextures)

for textureasset in assetlist:
    # Our TextureAsset is an AssetData object. We want a Texture2D object
    # Ensure it's a Texture2D
    if unreal.Texture2D.static_class() == texture.get_class():
        texture = textureasset.get_asset()
        
        # These are equivalent
        #print texture.mip_gen_settings
        #print texture.get_editor_property("mip_gen_settings")
        
        # Set our LOD Group to  TEXTUREGROUP_UI(15)
        texturelodgroup = unreal.TextureGroup.cast(16)
        if texture.get_editor_property("lod_group") != texturelodgroup:
            texture.set_editor_property("lod_group", texturelodgroup)
        
        # Set our CompressionSetting to UserInterface2D (7)
        texturecompressionsetting = unreal.TextureCompressionSettings.cast(7)
        if texture.get_editor_property("compression_settings") != texturecompressionsetting:
            texture.set_editor_property("compression_settings", texturecompressionsetting)
            count = count + 1
        
        # Set our MipGenSetting to NoMipMaps (13)
        texturemipsetting = unreal.TextureMipGenSettings.cast(13)
        if texture.get_editor_property("mip_gen_settings") != texturemipsetting:
            texture.set_editor_property("mip_gen_settings", texturemipsetting)
        
        # Set our NeverStream property to True
        if texture.get_editor_property("never_stream") == False:
            texture.set_editor_property("never_stream", True)
        
        # Save the asset (path, True == only_if_is_dirty)
        unreal.EditorAssetLibrary.save_asset(textureasset.get_full_name(), True)

#end For-loop