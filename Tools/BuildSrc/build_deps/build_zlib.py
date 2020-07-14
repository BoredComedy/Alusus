import os
import sys
import multiprocessing
import tarfile
import shutil
import subprocess
import wget
import platform
import stat
SOURCE_LOCATION = os.path.abspath(__file__)
sys.path.insert(0, os.path.dirname(os.path.dirname(SOURCE_LOCATION)))
from msg import info_msg, success_msg, fail_msg  # noqa
from build_deps import template_build  # noqa
from utils import get_host_cxx_arch, unix_copy2, shell_join, shell_split, which  # noqa
from custom_cc_cxx import create_new_environ_with_custom_cc_cxx  # noqa


_ZLIB_SRC_URL = "https://sourceforge.net/projects/libpng/files/zlib/1.2.11/zlib-1.2.11.tar.gz"


class build_zlib(template_build.template_build):
    def _check_built(install_path, target_system=None):
        # Check for unfinished RPATHs updating process exists.
        if os.path.exists(os.path.join(os.environ["ALUSUS_TMP_DIR"], "UPDATING_RPATHS")):
            return False
        if target_system == "windows" or platform.system == "Windows" and not target_system:
            return os.path.exists(os.path.join(install_path["root"], install_path["lib"], "libzlib.dll.a")) and\
                os.path.exists(os.path.join(install_path["root"], install_path["lib"], "libz.dll.a")) and\
                os.path.exists(os.path.join(
                    install_path["root"], install_path["bin"], "libzlib.dll"))
        elif target_system == "linux" or platform.system() == "Linux" and not target_system:
            return os.path.exists(os.path.join(install_path["root"], install_path["lib"], "libz.so.1.2.11")) and\
                os.path.exists(os.path.join(install_path["root"], install_path["lib"], "libz.so.1")) and\
                os.path.exists(os.path.join(
                    install_path["root"], install_path["lib"], "libz.so"))
        elif target_system == "macos" or platform.system() == "Darwin" and not target_system:
            return os.path.exists(os.path.join(install_path["root"], install_path["lib"], "libz.1.2.11.dylib")) and\
                os.path.exists(os.path.join(install_path["root"], install_path["lib"], "libz.1.dylib")) and\
                os.path.exists(os.path.join(
                    install_path["root"], install_path["lib"], "libz.dylib"))
        return False

    def build(deps_path, install_path, num_threads=multiprocessing.cpu_count(), target_system=None):
        global _ZLIB_SRC_URL

        info_msg("Building zlib 1.2.11...")

        if build_zlib._check_built(install_path, target_system=target_system):
            success_msg("Building zlib 1.2.11.")
            return True

        os.makedirs(deps_path, exist_ok=True)
        os.makedirs(install_path["root"], exist_ok=True)
        original_dir = os.getcwd()
        os.chdir(deps_path)

        # Download zlib.
        if not os.path.exists(os.path.join("zlib-1.2.11.src", "EXTRACTED")):
            info_msg("Getting zlib 1.2.11 sources...")
            try:
                os.remove("zlib-1.2.11.tar.gz")
            except FileNotFoundError:
                pass
            try:
                shutil.rmtree("zlib-1.2.11.src")
            except FileNotFoundError:
                pass
            try:
                shutil.rmtree("zlib-1.2.11")
            except FileNotFoundError:
                pass
            wget.download(_ZLIB_SRC_URL)
            # Print new line after "wget.download", as the latter method will not do so.
            print("")
            with tarfile.open("zlib-1.2.11.tar.gz") as fd:
                fd.extractall(".")
            os.rename("zlib-1.2.11", "zlib-1.2.11.src")
            try:
                os.remove("zlib-1.2.11.tar.gz")
            except FileNotFoundError:
                pass
            with open(os.path.join("zlib-1.2.11.src", "EXTRACTED"), "w") as fd:
                fd.write("EXTRACTED CHECKER")
            success_msg("zlib 1.2.11 sources have been downloaded.")
        else:
            success_msg("zlib 1.2.11 sources are already downloaded.")

        # Build zlib.
        if target_system != None:
            # Cross compile.

            # Build zlib for the host first.
            info_msg("Building zlib 1.2.11 for the host...")
            os.makedirs("zlib-1.2.11.host.build", exist_ok=True)
            os.makedirs("zlib-1.2.11.host.install", exist_ok=True)
            os.chdir("zlib-1.2.11.host.build")
            new_environ = os.environ.copy()
            new_environ["CC"] = which(
                new_environ["ALUSUS_HOST_CC"] if "ALUSUS_HOST_CC" in new_environ else "clang")
            new_environ["CXX"] = which(
                new_environ["ALUSUS_HOST_CXX"] if "ALUSUS_HOST_CXX" in new_environ else "clang++")
            new_environ["AR"] = which(
                new_environ["ALUSUS_HOST_AR"] if "ALUSUS_HOST_AR" in new_environ else "ar")
            new_environ["AS"] = which(
                new_environ["ALUSUS_HOST_AS"] if "ALUSUS_HOST_AS" in new_environ else "as")
            new_environ["RANLIB"] = which(
                new_environ["ALUSUS_HOST_RANLIB"] if "ALUSUS_HOST_RANLIB" in new_environ else "ranlib")
            new_environ["NM"] = which(
                new_environ["ALUSUS_HOST_NM"] if "ALUSUS_HOST_NM" in new_environ else "nm")
            new_environ["STRIP"] = which(
                new_environ["ALUSUS_HOST_STRIP"] if "ALUSUS_HOST_STRIP" in new_environ else "strip")
            new_environ["LD"] = which(
                new_environ["ALUSUS_HOST_LD"] if "ALUSUS_HOST_LD" in new_environ else "ld")
            if platform.system() == "Windows":
                new_environ["RC"] = which(
                    new_environ["ALUSUS_HOST_RC"] if "ALUSUS_HOST_RC" in new_environ else "windres")
            if platform.system() == "Darwin":
                new_environ["INSTALL_NAME_TOOL"] = which(
                    new_environ["ALUSUS_HOST_INSTALL_NAME_TOOL"] if "ALUSUS_HOST_INSTALL_NAME_TOOL" in new_environ else "install_name_tool")
                new_environ["OTOOL"] = which(
                    new_environ["ALUSUS_HOST_OTOOL"] if "ALUSUS_HOST_OTOOL" in new_environ else "otool")
            if "ALUSUS_HOST_LIBRARY_PATH" in new_environ:
                new_environ["LIBRARY_PATH"] = new_environ["ALUSUS_HOST_LIBRARY_PATH"]
            else:
                try:
                    del new_environ["LIBRARY_PATH"]
                except KeyError:
                    pass
            if "ALUSUS_HOST_CPATH" in new_environ:
                new_environ["CPATH"] = new_environ["ALUSUS_HOST_CPATH"]
            else:
                try:
                    del new_environ["CPATH"]
                except KeyError:
                    pass
            if "ALUSUS_HOST_C_INCLUDE_PATH" in new_environ:
                new_environ["C_INCLUDE_PATH"] = new_environ["ALUSUS_HOST_C_INCLUDE_PATH"]
            else:
                try:
                    del new_environ["C_INCLUDE_PATH"]
                except KeyError:
                    pass
            if "ALUSUS_HOST_CPLUS_INCLUDE_PATH" in new_environ:
                new_environ["CPLUS_INCLUDE_PATH"] = new_environ["ALUSUS_HOST_CPLUS_INCLUDE_PATH"]
            else:
                try:
                    del new_environ["CPLUS_INCLUDE_PATH"]
                except KeyError:
                    pass
            if "ALUSUS_HOST_PKG_CONFIG_PATH" in new_environ:
                new_environ["PKG_CONFIG_PATH"] = new_environ["ALUSUS_HOST_PKG_CONFIG_PATH"]
            else:
                try:
                    del new_environ["PKG_CONFIG_PATH"]
                except KeyError:
                    pass
            host_target_system = "windows" if (platform.system() == "Windows") else (
                "linux" if (platform.system() == "Linux") else "macos")
            new_environ = create_new_environ_with_custom_cc_cxx(
                new_environ, target_system=host_target_system)
            cmake_cmd = ["cmake",
                         os.path.join(deps_path, "zlib-1.2.11.src"),
                         "-DCMAKE_BUILD_TYPE=Release",
                         "-DCMAKE_INSTALL_PREFIX={}".format(
                             os.path.join(deps_path, "zlib-1.2.11.host.install")),
                         "-DCMAKE_RANLIB={}".format(which(
                             new_environ["RANLIB"] if "RANLIB" in new_environ else "ranlib")),
                         "-DCMAKE_AR={}".format(which(
                             new_environ["AR"] if "AR" in new_environ else "ar")),
                         "-DCMAKE_LINKER={}".format(which(
                             new_environ["LD"] if "LD" in new_environ else "ld")),
                         "-DCMAKE_STRIP={}".format(which(new_environ["STRIP"] if "STRIP" in new_environ else "strip"))]
            if platform.system() == "Darwin":
                cmake_cmd += [
                    "-DCMAKE_INSTALL_NAME_TOOL={}".format(which(
                        new_environ["INSTALL_NAME_TOOL"] if "INSTALL_NAME_TOOL" in new_environ else "install_name_tool")),
                    "-DCMAKE_OTOOL={}".format(which(
                        new_environ["OTOOL"] if "OTOOL" in new_environ else "otool"))
                ]
            if platform.system() == "Windows":
                cmake_cmd += [
                    "-DCMAKE_RC_COMPILER={}".format(which(
                        new_environ["RC"] if "RC" in new_environ else "windres")),
                    "-G", "MinGW Makefiles",
                    "-DCMAKE_SH=CMAKE_SH-NOTFOUND"
                ]
            p = subprocess.Popen(cmake_cmd, env=new_environ)
            ret = p.wait()
            if ret:
                fail_msg("Building zlib 1.2.11.")
                os.chdir(original_dir)
                return False
            p = subprocess.Popen(
                [("mingw32-make" if (platform.system() == "Windows")
                  else "make"), "-j{}".format(num_threads), "install"],
                env=new_environ)
            ret = p.wait()
            if ret:
                fail_msg("Building zlib 1.2.11.")
                os.chdir(original_dir)
                return False

            # Build zlib for the target now.
            new_environ = os.environ.copy()
            new_environ = create_new_environ_with_custom_cc_cxx(
                new_environ, target_system=target_system)
            host_cxx_arch = get_host_cxx_arch()
            info_msg("Building zlib 1.2.11 for the target...")
            os.chdir("..")
            os.makedirs("zlib-1.2.11.target.build", exist_ok=True)
            os.makedirs("zlib-1.2.11.target.install", exist_ok=True)
            os.chdir("zlib-1.2.11.target.build")
            cmake_system_name = "Windows" if (target_system == "windows") else (
                "Darwin" if (target_system == "macos") else "Linux")
            cmake_cmd += [
                "-DCMAKE_CROSSCOMPILING=TRUE",
                "-DCMAKE_SYSTEM_NAME={}".format(cmake_system_name),
                "-DCMAKE_SYSTEM_PROCESSOR={}".format(host_cxx_arch)
            ]
            cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_INSTALL_PREFIX")][0]] =\
                "-DCMAKE_INSTALL_PREFIX={}".format(
                    os.path.join(deps_path, "zlib-1.2.11.target.install"))
            cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_RANLIB")][0]] =\
                "-DCMAKE_RANLIB={}".format(
                    which(new_environ["RANLIB"] if "RANLIB" in new_environ else "ranlib"))
            cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_AR")][0]] =\
                "-DCMAKE_AR={}".format(
                    which(new_environ["AR"] if "AR" in new_environ else "ar"))
            cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_LINKER")][0]] =\
                "-DCMAKE_LINKER={}".format(
                    which(new_environ["LD"] if "LD" in new_environ else "ld"))
            cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_STRIP")][0]] =\
                "-DCMAKE_STRIP={}".format(
                    which(new_environ["STRIP"] if "STRIP" in new_environ else "strip"))
            if target_system == "macos":
                if platform.system() == "Darwin":
                    cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_INSTALL_NAME_TOOL")][0]] =\
                        "-DCMAKE_INSTALL_NAME_TOOL={}".format(
                            which(new_environ["INSTALL_NAME_TOOL"] if "INSTALL_NAME_TOOL" in new_environ else "install_name_tool"))
                    cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_OTOOL")][0]] =\
                        "-DCMAKE_OTOOL={}".format(
                            which(new_environ["OTOOL"] if "OTOOL" in new_environ else "otool"))
                else:
                    cmake_cmd += [
                        "-DCMAKE_INSTALL_NAME_TOOL={}".format(
                            which(new_environ["INSTALL_NAME_TOOL"] if "INSTALL_NAME_TOOL" in new_environ else "install_name_tool")),
                        "-DCMAKE_OTOOL={}".format(
                            which(new_environ["OTOOL"] if "OTOOL" in new_environ else "otool"))
                    ]
            if target_system == "windows":
                if platform.system() == "Windows":
                    cmake_cmd[[cmake_cmd.index(item) for item in cmake_cmd if item.startswith("-DCMAKE_RC_COMPILER")][0]] =\
                        "-DCMAKE_RC_COMPILER={}".format(
                            which(new_environ["RC"] if "RC" in new_environ else "windres"))
                else:
                    cmake_cmd += [
                        "-DCMAKE_RC_COMPILER={}".format(
                            which(new_environ["RC"] if "RC" in new_environ else "windres"))
                    ]
            p = subprocess.Popen(cmake_cmd, env=new_environ)
            ret = p.wait()
            if ret:
                fail_msg("Building zlib 1.2.11.")
                os.chdir(original_dir)
                return False
            p = subprocess.Popen(
                [("mingw32-make" if (platform.system() == "Windows") else "make"), "-j{}".format(num_threads), "install"], env=new_environ)
            ret = p.wait()
            if ret:
                fail_msg("Building zlib 1.2.11.")
                os.chdir(original_dir)
                return False

        else:
            # Build zlib for the host only.
            new_environ = os.environ.copy()
            new_environ = create_new_environ_with_custom_cc_cxx(
                new_environ, target_system=target_system)
            info_msg("Building zlib 1.2.11 for the host...")
            os.makedirs("zlib-1.2.11.build", exist_ok=True)
            os.makedirs("zlib-1.2.11.install", exist_ok=True)
            os.chdir("zlib-1.2.11.build")
            cmake_cmd = ["cmake",
                         os.path.join(deps_path, "zlib-1.2.11.src"),
                         "-DCMAKE_BUILD_TYPE=Release",
                         "-DCMAKE_INSTALL_PREFIX={}".format(
                             os.path.join(deps_path, "zlib-1.2.11.install")),
                         "-DCMAKE_RANLIB={}".format(
                             which(new_environ["RANLIB"] if "RANLIB" in new_environ else "ranlib")),
                         "-DCMAKE_AR={}".format(
                             which(new_environ["AR"] if "AR" in new_environ else "ar")),
                         "-DCMAKE_LINKER={}".format(
                             which(new_environ["LD"] if "LD" in new_environ else "ld")),
                         "-DCMAKE_STRIP={}".format(which(new_environ["STRIP"] if "STRIP" in new_environ else "strip"))]
            if platform.system() == "Darwin":
                cmake_cmd += [
                    "-DCMAKE_INSTALL_NAME_TOOL={}".format(which(
                        new_environ["INSTALL_NAME_TOOL"] if "INSTALL_NAME_TOOL" in new_environ else "install_name_tool")),
                    "-DCMAKE_OTOOL={}".format(which(
                        new_environ["OTOOL"] if "OTOOL" in new_environ else "otool"))
                ]
            if platform.system() == "Windows":
                cmake_cmd += [
                    "-DCMAKE_RC_COMPILER={}".format(which(
                        new_environ["RC"] if "RC" in new_environ else "windres")),
                    "-G", "MinGW Makefiles",
                    "-DCMAKE_SH=CMAKE_SH-NOTFOUND"
                ]
            p = subprocess.Popen(cmake_cmd, env=new_environ)
            ret = p.wait()
            if ret:
                fail_msg("Building zlib 1.2.11.")
                os.chdir(original_dir)
                return False
            p = subprocess.Popen(
                [("mingw32-make" if (platform.system() == "Windows") else "make"), "-j{}".format(num_threads), "install"], env=new_environ)
            ret = p.wait()
            if ret:
                fail_msg("Building zlib 1.2.11.")
                os.chdir(original_dir)
                return False

        os.makedirs(os.path.join(
            install_path["root"], install_path["lib"]), exist_ok=True)
        os.makedirs(os.path.join(
            install_path["root"], install_path["bin"]), exist_ok=True)
        if target_system == "windows" or platform.system() == "Windows" and not target_system:
            shutil.copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "windows") else "zlib-1.2.11.install"), "lib", "libzlib.dll.a"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libzlib.dll.a")
            )
            shutil.copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "windows") else "zlib-1.2.11.install"), "lib", "libzlib.dll.a"),
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "windows") else "zlib-1.2.11.install"), "lib", "libz.dll.a"),
            )
            shutil.copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "windows") else "zlib-1.2.11.install"), "lib", "libz.dll.a"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libz.dll.a"),
            )
            shutil.copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "windows") else "zlib-1.2.11.install"), "bin", "libzlib.dll"),
                os.path.join(install_path["root"],
                             install_path["bin"], "libzlib.dll")
            )
        elif target_system == "linux" or platform.system() == "Linux" and not target_system:
            unix_copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "linux") else "zlib-1.2.11.install"), "lib", "libz.so.1.2.11"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libz.so.1.2.11")
            )
            unix_copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "linux") else "zlib-1.2.11.install"), "lib", "libz.so.1"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libz.so.1")
            )
            unix_copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "linux") else "zlib-1.2.11.install"), "lib", "libz.so"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libz.so")
            )
        elif target_system == "macos" or platform.system() == "Darwin" and not target_system:
            unix_copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "macos") else "zlib-1.2.11.install"), "lib", "libz.1.2.11.dylib"),
                os.path.join(
                    install_path["root"], install_path["lib"], "libz.1.2.11.dylib")
            )
            unix_copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "macos") else "zlib-1.2.11.install"), "lib", "libz.1.dylib"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libz.1.dylib")
            )
            unix_copy2(
                os.path.join(
                    deps_path, ("zlib-1.2.11.target.install" if (target_system == "macos") else "zlib-1.2.11.install"), "lib", "libz.dylib"),
                os.path.join(install_path["root"],
                             install_path["lib"], "libz.dylib")
            )

        success_msg("Building zlib 1.2.11.")
        os.chdir(original_dir)
        return True

    def get_dep_environ(deps_path, target_system=None):
        host_sep = ":" if platform.system() != "Windows" else ";"
        new_environ = os.environ.copy()
        if target_system != None:
            new_environ["ALUSUS_HOST_PATH"] = os.path.join(deps_path, "zlib-1.2.11.host.install", "bin") +\
                ("" if ("ALUSUS_HOST_PATH" not in new_environ) else
                    (host_sep + new_environ["ALUSUS_HOST_PATH"]))
            new_environ["ALUSUS_HOST_LD_LIBRARY_PATH"] = os.path.join(deps_path, "zlib-1.2.11.host.install", "lib") +\
                ("" if ("ALUSUS_HOST_LD_LIBRARY_PATH" not in new_environ) else
                    (host_sep + new_environ["ALUSUS_HOST_LD_LIBRARY_PATH"]))
            new_environ["ALUSUS_HOST_LIBRARY_PATH"] = os.path.join(deps_path, "zlib-1.2.11.host.install", "lib") +\
                ("" if ("ALUSUS_HOST_LIBRARY_PATH" not in new_environ) else
                 (host_sep + new_environ["ALUSUS_HOST_LIBRARY_PATH"]))
            new_environ["ALUSUS_HOST_CPATH"] = os.path.join(deps_path, "zlib-1.2.11.host.install", "include") +\
                ("" if ("ALUSUS_HOST_CPATH" not in new_environ) else
                 (host_sep + new_environ["ALUSUS_HOST_CPATH"]))
            new_environ["ALUSUS_HOST_PKG_CONFIG_PATH"] = os.path.join(deps_path, "zlib-1.2.11.host.install", "share", "pkgconfig") +\
                ("" if ("ALUSUS_HOST_PKG_CONFIG_PATH" not in new_environ) else
                 (host_sep + new_environ["ALUSUS_HOST_PKG_CONFIG_PATH"]))
            new_environ["CPATH"] = os.path.join(deps_path, "zlib-1.2.11.target.install", "include") +\
                ("" if ("CPATH" not in new_environ) else
                 (host_sep + new_environ["CPATH"]))
            new_environ["LIBRARY_PATH"] = os.path.join(deps_path, "zlib-1.2.11.target.install", "lib") +\
                ("" if ("LIBRARY_PATH" not in new_environ) else
                 (host_sep + new_environ["LIBRARY_PATH"]))
            new_environ["PKG_CONFIG_PATH"] = os.path.join(deps_path, "zlib-1.2.11.target.install", "share", "pkgconfig") +\
                ("" if ("PKG_CONFIG_PATH" not in new_environ) else
                 (host_sep + new_environ["PKG_CONFIG_PATH"]))
            new_environ["ZLIB_ROOT_DIR"] = os.path.join(
                deps_path, "zlib-1.2.11.target.install")
        else:
            new_environ["CPATH"] = os.path.join(deps_path, "zlib-1.2.11.install", "include") +\
                ("" if ("CPATH" not in new_environ) else
                 (host_sep + new_environ["CPATH"]))
            new_environ["LIBRARY_PATH"] = os.path.join(deps_path, "zlib-1.2.11.install", "lib") +\
                ("" if ("LIBRARY_PATH" not in new_environ) else
                 (host_sep + new_environ["LIBRARY_PATH"]))
            new_environ["PKG_CONFIG_PATH"] = os.path.join(deps_path, "zlib-1.2.11.install", "share", "pkgconfig") +\
                ("" if ("PKG_CONFIG_PATH" not in new_environ) else
                 (host_sep + new_environ["PKG_CONFIG_PATH"]))
        return new_environ