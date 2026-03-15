for file in $(find ./build -type f -name "*.vert"); do glslc $file -o $file.spv; done
for file in $(find ./build -type f -name "*.frag"); do glslc $file -o $file.spv; done
