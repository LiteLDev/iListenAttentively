add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

add_requires("levilamina 26.40.*", {configs = {target_type = get_config("target_type")}})
add_requires("levibuildscript")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")

target("iListenAttentively")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE", "ILA_EXPORT",
        "_HAS_CXX23=1")
        set_exceptions("none") -- To avoid conflicts with /EHa.
        add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            {tools = {"clang_cl"}}
        )
        set_toolchains("clang-cl")
    end
    set_optimize("aggressive")
    set_configdir("$(builddir)/config")
    set_configvar("IL_WORKSPACE_FOLDER", "$(projectdir)")
    add_configfiles("src/(ila/**.h.in)")
    add_files("src/ila/**.cpp")
    add_files("src/ila/**.rc")
    add_headerfiles("src/(ila/**.h)")
    add_includedirs("src", "$(builddir)/config")
    add_packages(
		"levilamina",
        "fmt",
        "magic_enum",
        "nlohmann_json"
    )
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")

    if is_mode("debug") then
        add_defines("ILA_DEBUG")
    end

    if has_config("tests") then
        add_defines("ILA_TESTS")
        add_includedirs("src-test/")
        add_files("src-test/**.cpp")
        before_build(function (target)
            local include_all = "#pragma once\n"
            for _, filepath in ipairs(os.files("src/ila/event/**.h")) do
                include_all = include_all .. "\n#include \"" .. path.relative(filepath, "src") .. "\""
            end
            io.writefile("src-test/include_all.cpp", include_all)
            io.gsub("src-test/include_all.cpp", "\\", "/")
        end)
    end

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
