add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("lite_pdb")
    set_default(true)
    set_showmenu(true)
    set_description("Enable Lite PDB")
option_end()

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")
option_end()

option("fakes")
    set_default(false)
    set_showmenu(true)
    set_description("Enable fakes")
option_end()

add_requires("levilamina 26.20.0", { configs = { target_type = get_config("target_type") } })

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
    add_shflags(
        "/DELAYLOAD:bedrock_runtime.dll"
    )
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "ILA_EXPORT",
        "_HAS_CXX23=1",
        "_SILENCE_CXX20_IS_ALWAYS_EQUAL_DEPRECATION_WARNING=1",
        ( is_config("target_type", "server") and "LL_PLAT_S" or "LL_PLAT_C" ),
        ( is_mode("debug") and "ILA_DEBUG" or "ILA_RELEASE" )
    )
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
    set_configdir("$(builddir)/config")
    add_files("src/**.rc")
    add_includedirs("$(builddir)/config")
    add_packages("levilamina")
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")
    add_syslinks("ws2_32")
    if is_mode("release") then
        set_strip("all")
        set_optimize("aggressive")
    else
        remove_files("src/common/ila/core/SymbolProvider.cpp")
    end
    local function compute_symbol_provider_key(version_info)
        local seed = tonumber((version_info.commit_hash or ""):sub(1, 2), 16)
        if not seed then return 105 end
        return (seed % 254) + 1
    end
    local symbolProviderKey = 105

    add_files("src/common/**.cpp")
    add_includedirs("src/common")
    add_headerfiles("src/common/**.h", "src/common/**.hpp")
    add_configfiles("src/common/(**.h.in)")

    local target_type = get_config("target_type") or "server"
    add_files("src/" .. target_type .. "/**.cpp")
    add_includedirs("src/" .. target_type)
    add_headerfiles("src/" .. target_type .. "/**.h", "src/" .. target_type .. "/**.hpp")
    add_configfiles("src/" .. target_type .. "/(**.h.in)")

    if has_config("tests") then
        add_defines("ILA_TESTS")
        add_files("src-test/common/**.cpp")
        add_includedirs("src-test/common/")
        add_headerfiles("src-test/common/**.h")

        add_files("src-test/" .. target_type .. "/**.cpp")
        add_includedirs("src-test/" .. target_type .. "/")
        add_headerfiles("src-test/" .. target_type .. "/**.h")
    end

    if has_config("fakes") then
        add_defines("ILA_FAKES")
        add_files("src-fakes/**.cpp")
        add_includedirs("src-fakes/")
        add_headerfiles("src-fakes/**.h")
    end

    on_load(function (target)
        local version_info = import("scripts.get-version-info", { rootdir = os.projectdir() }).get_version_info()
        symbolProviderKey = compute_symbol_provider_key(version_info)
        target:set("configvar", "ILA_VERSION_MAJOR", version_info.major)
        target:set("configvar", "ILA_VERSION_MINOR", version_info.minor)
        target:set("configvar", "ILA_VERSION_PATCH", version_info.patch)
        target:set("configvar", "ILA_SYMBOL_PROVIDER_KEY", symbolProviderKey)
        if version_info.prerelease then
            target:set("configvar", "ILA_VERSION_PRERELEASE", version_info.prerelease)
        end
    end)

    before_link(function(target)
        import("lib.detect.find_file")
        import("core.project.config")

        -- 修复莫名其妙的环境变量缺失导致的链接失败
        os.addenvs(target:pkgenvs())

        local libdir = path.join(config.builddir(), ".prelink", "lib")
        if os.exists(libdir) then os.rm(libdir) end
        os.mkdir(libdir)

        local data = assert(find_file("bedrock_runtime_data", {"$(env PATH)"}), "Cannot find bedrock_runtime_data")
        local link = assert(find_file("prelink.exe", {"$(env PATH)"}), "Cannot find prelink.exe")

        os.runv(link, {
            string.format("%s-%s-%s", get_config("target_type"), target:plat(), target:arch()),
            path.join(config.builddir(), ".prelink"),
            data,
            table.unpack(target:objectfiles())
        })

        target:add("linkdirs", libdir)
        target:add("links", "bedrock_runtime_api")
    end)

    after_build(function (target)
        local output_dir = path.join(os.projectdir(), "bin", "dll", target:name())
        local artifact_file = target:targetfile()

        os.rm(output_dir)

        if is_mode("release") then
            os.runv(
                "python.exe",
                {
                    path.join(os.projectdir(), "tools", "iEncryptImports.py"),
                    artifact_file,
                    symbolProviderKey
                }
            )
        end

        os.vcp(artifact_file, format("%s/", output_dir))
        os.vcp(target:symbolfile(), format("%s/../../pdb/", output_dir))
        if has_config("lite_pdb") then
            os.run(path.join(os.projectdir(), "tools", "iLitePDB.exe"))
        else 
            os.vcp(target:symbolfile(), output_dir)
        end

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

        for _, header_file in ipairs(target:headerfiles()) do
            if not header_file:endswith(".i.h") then
                os.vcp(
                    header_file,
                    path.join(
                        output_dir,
                        "include",
                        (path.relative(header_file, os.projectdir()):gsub("^[^/\\]+[/\\][^/\\]+[/\\]", ""))
                    )
                )
            end
        end

        for _, source_file in ipairs(target:sourcefiles()) do
            if source_file:endswith(".open.cpp") then
                os.vcp(
                    source_file,
                    path.join(
                        output_dir,
                        "src",
                        ((path.relative(source_file, os.projectdir()):gsub("^[^/\\]+[/\\]", "")):gsub("%.open%.cpp$", ".cpp"))
                    )
                )
            end
        end

        local target_implib = target:artifactfile("implib")
        if target_implib and os.isfile(target_implib) then
            os.vcp(target_implib, format("%s/lib/", output_dir))
        end
    end)
