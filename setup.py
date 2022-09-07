# -*- coding: utf-8 -*-
import os
import re
import subprocess
import sys

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import codecs

# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # required for auto-detection & inclusion of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={}".format(extdir),
            "-DPYTHON_EXECUTABLE={}".format(sys.executable),
            "-DCMAKE_BUILD_TYPE={}".format(cfg),  # not used on MSVC, but no harm
            "-DBUILD_TESTING=off"
        ]
        build_args = []
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        cmake_args += [
            "-DVERSION_INFO={}".format(self.distribution.get_version())
        ]

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator:
                try:
                    import ninja  # noqa: F401

                    cmake_args += ["-GNinja"]
                except ImportError:
                    pass

        else:

            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}".format(cfg.upper(), extdir)
                ]
                build_args += ["--config", cfg]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += ["-j{}".format(self.parallel)]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp
        )
        subprocess.check_call(
            ["cmake", "--build", ".", "--target", "quandelibc"] + build_args, cwd=self.build_temp
        )

def read(rel_path):
    here = os.path.abspath(os.path.dirname(__file__))
    try:
        with codecs.open(os.path.join(here, rel_path), 'r') as fp:
            return fp.read()
    except:
        return ""

def get_version(rel_path):
    for line in read(rel_path).splitlines():
        if line.startswith('Version:'):
            delim = ' '
            return line.split(delim)[1]
    else:
        return '0.0.0'

setup(
    name="quandelibc",
    author="Perceval.OSS@Quandela.com",
    author_email="Perceval.OSS@Quandela.com",
    description="Optimized C-functions for Perceval",
    long_description="Collection of c-optimized computation primitive available as python module. When possible, the low level function uses INTEL SIMD primitives.",
    project_urls={
        "Documentation" : "https://perceval.quandela.net/docs-quandelibc/",
        "Source": "https://github.com/Quandela/QuandeLibC",
        "Tracker": "https://github.com/Quandela/QuandeLibC/issues"
    },
    url="https://github.com/Quandela/QuandeLibC",
    download_url="https://github.com/Quandela/QuandeLibC",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License"
    ],
    setup_requires=["scmver", "wheel"],
    scmver=get_version('PKG-INFO')=='0.0.0',
    version=get_version('PKG-INFO'),
    ext_modules=[CMakeExtension("quandelibc")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    extras_require={"test": ["pytest"]},
)
