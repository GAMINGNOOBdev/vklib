Get-ChildItem -Path .\build -Recurse -Filter "*.vert" | ForEach-Object { glslc $_.FullName -o "$($_.FullName).spv" }
Get-ChildItem -Path .\build -Recurse -Filter "*.frag" | ForEach-Object { glslc $_.FullName -o "$($_.FullName).spv" }
