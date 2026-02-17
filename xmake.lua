add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")
option_end()

add_requires("levilamina 1.9.5", { configs = { target_type = get_config("target_type") } })
add_requires("levibuildscript 0.5.0")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

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
        "_HAS_CXX23=1",
        ( is_config("target_type", "server") and "LL_PLAT_S" or "LL_PLAT_C" ),
        ( is_mode("debug") and "ILA_DEBUG" or "ILA_RELEASE" )
    )
    set_optimize("aggressive")
    set_configdir("$(builddir)/config")
    add_configfiles("src/(ila/**.h.in)")
    add_includedirs("$(builddir)/config")
    add_packages("levilamina")
    add_rules("@levibuildscript/linkrule")
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")

    add_files("src/common/ila/**.cpp")
    add_includedirs("src/common")
    add_headerfiles("src/common/ila/**.h")
    add_headerfiles("src/common/ila/**.hpp")
    if is_config("target_type", "server") then
        add_files("src/server/ila/**.cpp")
        add_includedirs("src/server")
        add_headerfiles("src/server/ila/**.h")
        add_headerfiles("src/server/ila/**.hpp")
    elseif is_config("target_type", "client") then
        add_files("src/client/ila/**.cpp")
        add_includedirs("src/client")
        add_headerfiles("src/client/ila/**.h")
        add_headerfiles("src/client/ila/**.hpp")
    end

    if has_config("tests") then
        add_defines("ILA_TESTS")
        add_files("src-test/common/**.cpp")
        add_includedirs("src-test/common/")
        add_headerfiles("src-test/common/**.h")
        if is_config("target_type", "server") then
            add_files("src-test/server/**.cpp")
            add_includedirs("src-test/server/")
            add_headerfiles("src-test/server/**.h")
        elseif is_config("target_type", "client") then
            add_includedirs("src-test/client/")
            add_files("src-test/client/**.cpp")
            add_headerfiles("src-test/client/**.h")
        end
    end

    on_load(function (target)
        local version_info = import("scripts.get-version-info", { rootdir = os.projectdir() }).get_version_info()
        target:set("configvar", "ILA_VERSION_MAJOR", version_info.major)
        target:set("configvar", "ILA_VERSION_MINOR", version_info.minor)
        target:set("configvar", "ILA_VERSION_PATCH", version_info.patch)
        if version_info.prerelease then
            target:set("configvar", "ILA_VERSION_PRERELEASE", version_info.prerelease)
        end
    end)

    after_build(function (target) 
        local output_dir = path.join(os.projectdir(), "bin", "dll", target:name())

        os.rm(output_dir)

        os.vcp(target:targetfile(), format("%s/", output_dir))
        os.vcp(target:symbolfile(), format("%s/../../pdb/", output_dir))

        import("scripts.generate-manifest", { rootdir = os.projectdir() }).generate_manifest(
            format("%s/manifest.json", output_dir),
            {
                name = "iListenAttentively",
                entry = "iListenAttentively.dll",
                version = import("scripts.get-version-info", { rootdir = os.projectdir() }).get_version_info().version_str,
                author = "MiracleForest",
                description = "iListenAttentively is a rich and modern LeviLamina Minecraft event library!",
                passive = not get_config("tests") and is_mode("release"),
                platform = get_config("target_type")
            }
        )
    end)

    on_package(function (target)
        local output_dir = path.join(os.projectdir(), "bin", "sdk")

        os.rm(output_dir)

        local srcheaders, dstheaders = target:headerfiles(format("%s/include/", output_dir))
        if srcheaders and dstheaders and #srcheaders > 0 then
            for index = 1, #srcheaders, 1 do
                os.vcp(srcheaders[index], dstheaders[index])
            end
        else
            os.mkdir(path.join(output_dir, "include"))
        end

        local target_implib = target:artifactfile("implib")
        if target_implib and os.isfile(target_implib) then
            os.vcp(target_implib, format("%s/lib/", output_dir))
        end
    end)