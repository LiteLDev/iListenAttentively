add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- Dependencies from liteldev-repo.
add_requires("levilamina 1.4.0")
add_requires("levibuildscript 0.4.1")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")

target("iListenAttentively")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204",
        "/Ob3",
        "/Zo-"
    )
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "ILA_EXPORT",
        "_HAS_CXX23=1"
    )
    set_optimize("aggressive")
    set_configdir("$(buildir)/config")
    set_configvar("IL_WORKSPACE_FOLDER", "$(projectdir)")
    add_configfiles("src/(ila/**.h.in)")
    add_files("src/ila/**.cpp")
    add_files("src/ila/**.rc")
    add_headerfiles("src/(ila/**.h)")
    add_includedirs("src", "$(buildir)/config")
    add_packages(
		"levilamina",
 		"fmt",
        "magic_enum",
        "nlohmann_json"
    )
    add_rules("@levibuildscript/linkrule")
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")

    if is_mode("debug") then
        add_defines("ILA_DEBUG")
    end

    if has_config("tests") then
        add_defines("ILA_TESTS")
        add_includedirs("src-test/")
        add_headerfiles("src-test/**.h")
        add_files("src-test/**.cpp")
    end

    after_build(function (target)
        local output_directory = path.join(os.projectdir(), "bin") -- total Output Path
        local dll_directory = path.join(output_directory, "DLL", target:name()) -- Plugin Body Output Path
        local pdb_directory = path.join(output_directory, "PDB") -- pdb output path
        local sdk_directory = path.join(output_directory, "SDK") -- sdk output path
        local library_directory = path.join(sdk_directory, "lib") -- lib output path
        local includes_directory = path.join(sdk_directory, "include") -- sdk header file output path

        local major, minor, patch, suffix = os.iorun("git describe --tags --abbrev=0 --always"):match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            major, minor, patch = 0, 0, 0
            print("Failed to parse version tag, using 0.0.0")
        end

        -- delete old compilation results
        if os.exists(output_directory) then
            os.rm(output_directory)
            cprint("${bright yellow}[Mod packed] ${bright green}old compilations have been removed.")
        end

        -- generate the manifest.json file
        if not os.isfile(path.join(os.projectdir(), "manifest.json")) then
            return cprint("${bright yellow}[Mod packed] ${bright red}manifest.json does not exist!")
        end
        local manifest_path = path.join(dll_directory, "manifest.json")
        os.cp(path.join(os.projectdir(), "manifest.json"), manifest_path)
        local mod_define = {
            modName = target:name(),
            modFile = path.filename(target:targetfile()),
            modVersion = string.format("%d.%d.%d", major, minor, patch),
            passive = not has_config("tests")
        }
        io.gsub(manifest_path, "%${(.-)}", function(var)
            return tostring(mod_define[var]) or "${" .. var .. "}"
        end)
        cprint("${bright yellow}[Mod packed] ${bright green}has generated manifest.json to ${bright cyan}" .. manifest_path)

        -- copy the plugin body
        os.cp(target:targetfile(), path.join(dll_directory, target:name() .. ".dll"))
        cprint("${bright yellow}[Mod packed] ${bright green}dll has copied to ${bright cyan}" .. path.join(dll_directory, target:name() .. ".dll"))

        -- copy PDB
        local pdb_path = path.join(pdb_directory, target:name() .. ".pdb")
        if os.isfile(target:symbolfile()) then
            os.cp(target:symbolfile(), pdb_path)
            cprint("${bright yellow}[Mod packed] ${bright green}pdb has copied to ${bright cyan}" .. pdb_path)
        end

        -- copy lib
        os.cp(
            path.join(
                path.directory(target:targetfile()),
                path.basename(target:targetfile()) .. ".lib"
            ), 
            path.join(
                library_directory, target:name() .. ".lib"
            )
        )
        cprint("${bright yellow}[Mod packed] ${bright green}library has copied to ${bright cyan}" .. library_directory)
 
        -- iterate over all header files
        for _, headerfile in ipairs(target:headerfiles()) do
            os.cp(headerfile, path.join(includes_directory, path.relative(headerfile, "src")))
        end 
        for _, headerfile in ipairs(target:configfiles()) do
            os.cp(
                path.join("$(buildir)/config", path.relative(string.sub(headerfile, 0, -4), "src")), 
                path.join(includes_directory, path.relative(string.sub(headerfile, 0, -4), "src"))
            )
        end
        cprint("${bright yellow}[Mod packed] ${bright green}header files has copied to ${bright cyan}" .. includes_directory)
    end)

    on_load(function (target)
        local major, minor, patch, suffix = os.iorun("git describe --tags --abbrev=0 --always"):match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        if suffix then
            prerelease = suffix:match("-(.*)")
            if prerelease then
                prerelease = prerelease:gsub("\n", "")
            end
            if prerelease then
                target:set("configvar", "IL_VERSION_PRERELEASE", prerelease)
            end
        end
        target:set("configvar", "IL_VERSION_MAJOR", major)
        target:set("configvar", "IL_VERSION_MINOR", minor)
        target:set("configvar", "IL_VERSION_PATCH", patch)
    end)

    before_build(function (target)
        local include_all = "#pragma once\n"
        for _, filepath in ipairs(os.files("src/ila/event/**.h")) do
            include_all = include_all .. "\n#include \"" .. path.relative(filepath, "src") .. "\""
        end
        io.writefile("src/ila/include_all.h", include_all)
        io.gsub("src/ila/include_all.h", "\\", "/")
    end)