setlocal enabledelayedexpansion

set argCount=0
for %%x in (%*) do (
    python %~p0image-to-heightmap.py %%~x
)

pause