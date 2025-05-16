# Near Chuckle
Far Cry's leaked source code ported to run on SDL2 and Linux. Thanks to [ugozapad](https://github.com/ugozapad)
and [q4a](https://github.com/q4a) for their earlier work.

Warning: The Linux port is still very, very early in development and will probably crash. You can,
however, load a level, walk around, and shoot people. It is also highly recommended that
you apply the workaround patches in the Known Issues section to have a decent playing experience.

You need SDL2, OpenAL-Soft, and OpenGL to compile the port. You will also need
the Nvidia Cg toolkit, which is not included, to compile the OpenGL renderer. You can get it here:
https://developer.nvidia.com/cg-toolkit-download

Modify the `CMakeLists.txt` file in `RenderDll/XRenderOGL` and set `CG_LIB_PATH` to
the path containing `libCgGL.so` and `libCg.so`.

After building, place all of the .so files and the `FarCry` binary in a folder in Far Cry's installation folder (containing `FCData`, `Levels`, `Profiles`, `Shaders`). If you built with the supplied CMake Preset, they should be in `bin/x64-Debug`.
You can simply movie `x64-Debug` to the installation folder. Launch `FarCry` from inside the folder it is in.

## Known issues

### Broken decals

On my Linux machine, decal texture coordinates will frequently change every frame and look almost like
Z-fighting. They look fine on Windows when forcing the OpenGL renderer, so I am
not sure how to fix this. If decals look fine under your Linux machine, let me know!
Otherwise, a temporary workaround in the code is to disable rendering
anything that uses a specific render state in GLRendPipeline.cpp.

```
diff --git a/SourceCode/RenderDll/XRenderOGL/GLRendPipeline.cpp b/SourceCode/RenderDll/XRenderOGL/GLRendPipeline.cpp
index 5771c16..b019635 100644
--- a/SourceCode/RenderDll/XRenderOGL/GLRendPipeline.cpp
+++ b/SourceCode/RenderDll/XRenderOGL/GLRendPipeline.cpp
@@ -5614,6 +5614,12 @@ void CGLRenderer::EF_DrawGeneralPasses(SShaderTechnique *hs, SShader *ef, bool b
   int bFogOverrided = 0;
   SArrayPointer::m_LastEnabledPass = 0;
   slw = &hs->m_Passes[nStart];
+#if 1
+if ((slw->m_RenderState & GS_BLSRC_MASK) == GS_BLSRC_DSTCOL && (slw->m_RenderState & GS_BLDST_MASK) == GS_BLDST_ZERO)
+      {
+        return;
+      }
+#endif
   for (i=nStart; i<=nEnd; i++, slw++)
   {
     SArrayPointer::m_CurEnabledPass = 0;
```

### Crash when firing a weapon

For some reason, stack smashing occurs in the `CScriptObjectGame::SoundEvent` function. A current workaround
is to disable the function call in ScriptObjectGame.cpp.

```
diff --git a/SourceCode/CryGame/ScriptObjectGame.cpp b/SourceCode/CryGame/ScriptObjectGame.cpp
index 64fb351..d79388e 100644
--- a/SourceCode/CryGame/ScriptObjectGame.cpp
+++ b/SourceCode/CryGame/ScriptObjectGame.cpp
@@ -3571,7 +3571,7 @@ int CScriptObjectGame::SoundEvent(IFunctionHandler *pH)
 	CXClient *pCli=m_pGame->GetClient();
 	if(pCli)
 	{
-		pCli->SoundEvent((EntityId)nID,pos,fRadius,fThreat);
+		//pCli->SoundEvent((EntityId)nID,pos,fRadius,fThreat);
 	}
 	
 	return pH->EndFunction();
```