import re
import glob
import subprocess
from setuptools import setup, Extension

Mod_Sources = []
Mod_Include_Dirs = []
Mod_Library_Dirs = []
Mod_Libraries = []
Mod_Extra_Compile_Args = []

Mod_Sources += glob.glob("./src/*.cpp")
Mod_Sources += glob.glob("./src/Utils/*.cpp")
Mod_Sources += glob.glob("./src/Proto/*.cpp")

#print(Mod_Sources)

Result = subprocess.run(["pkg-config", "--cflags", "libdpdk"], text = True, capture_output = True)
Result = Result.stdout.rstrip("\n").split()
for str in Result:
    if (re.match(r"-I", str)):
        Mod_Include_Dirs.append(str[2:])

Mod_Extra_Compile_Args += "-include rte_config.h".split()
Mod_Extra_Compile_Args.append("-Wno-address-of-packed-member")
Mod_Extra_Compile_Args.append("-g")
Mod_Extra_Compile_Args.append("-O3")
Mod_Extra_Compile_Args.append("-march=native")
Mod_Extra_Compile_Args.append("-mrtm")
Mod_Extra_Compile_Args.append("-Wl,--as-needed")

Result = subprocess.run(["pkg-config", "--libs", "libdpdk"], text = True, capture_output = True)
Result = Result.stdout.rstrip("\n").split()
for str in Result:
    if (re.match(r"-L", str)):
        Mod_Library_Dirs.append(str[2:])
    elif (re.match(r"-l", str)):
        Mod_Libraries.append(str[2:])

setup (
    name = 'Pktgen_DPDK',
    version = "1.0",
    author = "Luan_233",
    author_email = "Luan_233@stu.xjtu.edu.cn",
    description = "Packet generator powered by DPDK.",
    url = "https://github.com/Luan-233/Pktgen-DPDK",
    ext_modules = [
        Extension(
            name = "Pktgen_DPDK",
            sources = Mod_Sources,
            language = 'c++',
            include_dirs = Mod_Include_Dirs,
            library_dirs = Mod_Library_Dirs,
            libraries = Mod_Libraries,
            extra_compile_args = Mod_Extra_Compile_Args
        )
    ]
)