import os
import shutil
Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}

env.Replace(PROGNAME="MMU_DIY-{0}".format(defines.get('VERSION')))