import os
import subprocess
import sys

source_shader_file = sys.argv[1]
output_dir = sys.argv[2]
output_file = os.path.join(
        output_dir,
        "{0}.spv".format(os.path.basename(source_shader_file)))

# Create the output directory if it doesn't exist.
# Because GN runs this script in parallel, this might fail even if the check
# is successful.
try:
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
except:
    pass


args = [
    "glslangValidator", "-V", source_shader_file, "-o", output_file,
]
subprocess.call(args)
