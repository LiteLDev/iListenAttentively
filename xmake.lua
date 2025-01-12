add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- Dependencies from xmake-repo.
add_requires("fmt 10.2.1")
add_requires("magic_enum v0.9.7")
add_requires("nlohmann_json v3.11.3")

-- Dependencies from liteldev-repo.
add_requires("levilamina 1.0.0-rc.2")
add_requires("levibuildscript 0.3.0")

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
        "/O2"
    )
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "ILA_EXPORT",
        "_HAS_CXX17",
        "_HAS_CXX20",
        "_HAS_CXX23"
    )
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
        local output_directory = path.join(os.projectdir(), "bin") -- 总输出路径
        local dll_directory = path.join(output_directory, "DLL", target:name()) -- 插件本体输出路径
        local pdb_directory = path.join(output_directory, "PDB") -- pdb输出路径
        local sdk_directory = path.join(output_directory, "SDK") -- sdk输出路径
        local library_directory = path.join(sdk_directory, "library") -- sdk库输出路径
        local includes_directory = path.join(sdk_directory, "includes") -- sdk头文件输出路径

        local major, minor, patch, suffix = os.iorun("git describe --tags --abbrev=0 --always"):match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            major, minor, patch = 0, 0, 0
            print("Failed to parse version tag, using 0.0.0")
        end

        -- 删除旧编译结果
        if os.exists(output_directory) then
            os.rm(output_directory)
            cprint("${bright yellow}[Mod打包] ${bright green}已删除旧编译产物")
        end

        -- 生成manifest.json文件
        if not os.isfile(path.join(os.projectdir(), "manifest.json")) then
            return cprint("${bright yellow}[Mod打包] ${bright red}manifest.json不存在！")
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
        cprint("${bright yellow}[Mod打包] ${bright green}已生成manifest.json至 ${bright cyan}" .. manifest_path)

        -- 复制插件本体
        os.cp(target:targetfile(), path.join(dll_directory, target:name() .. ".dll"))
        cprint("${bright yellow}[Mod打包] ${bright green}已复制DLL至           ${bright cyan}" .. path.join(dll_directory, target:name() .. ".dll"))

        -- 复制PDB
        local pdb_path = path.join(pdb_directory, target:name() .. ".pdb")
        if os.isfile(target:symbolfile()) then
            os.cp(target:symbolfile(), pdb_path)
            cprint("${bright yellow}[Mod打包] ${bright green}已复制PDB至           ${bright cyan}" .. pdb_path)
        end

        -- 复制library
        os.cp(
            path.join(
                path.directory(target:targetfile()),
                path.basename(target:targetfile()) .. ".lib"
            ), 
            path.join(
                library_directory, target:name() .. ".lib"
            )
        )
        cprint("${bright yellow}[Mod打包] ${bright green}已复制library至       ${bright cyan}" .. library_directory)
 
        -- 遍历所有头文件
        for _, headerfile in ipairs(target:headerfiles()) do
            os.cp(headerfile, path.join(includes_directory, path.relative(headerfile, "src")))
        end 
        for _, headerfile in ipairs(target:configfiles()) do
            os.cp(
                path.join("$(buildir)/config", path.relative(string.sub(headerfile, 0, -4), "src")), 
                path.join(includes_directory, path.relative(string.sub(headerfile, 0, -4), "src"))
            )
        end
        cprint("${bright yellow}[Mod打包] ${bright green}已复制头文件至        ${bright cyan}" .. includes_directory)
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